// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.
///
/// Synap command line test application for recursive NN.
///

#include "synap/network.hpp"
#include "synap/buffer.hpp"
#include "synap/arg_parser.hpp"
#include "synap/file_utils.hpp"

#include <iostream>
#include "float16.hpp"

using namespace std;
using namespace synaptics::synap;


int main(int argc, char** argv)
{
    ArgParser args(argc, argv, "Synap recursive network test program", "[options]");
    string model = args.get("-m", "<file> Model file (.synap or legacy .nb)");
    string nb = args.get("--nb", "<file> Deprecated, same as -m");
    string meta = args.get("--meta", "<file> json meta file for legacy .nb models");
    bool copy = args.has("--copy", "Copy output to 2nd input");
    bool swap = args.has("--swap", "Swap output and 2nd input buffers");
    int num_predict = max(stoi(args.get("-r", "<r> Repeat count", "9")), 9);
    args.check_help("--help", "Show help"
        "Three different modes are available:\n"
        "  share (default) -> output buffer is shared with input buffer (fastest)\n"
        "  copy -> output data is copied back to 2nd input (ok for small data)\n"
        "  swap -> output buffer is swapped with 2nd input buffer (ok for big data)\n"
    );
    validate_model_arg(model, nb, meta);
 
    // Create and load the model
    Network net;
    if (!net.load_model(model, meta)) {
        cerr << "Failed to load network" << endl;
        return 1;
    }
    if (net.inputs.size() != 2) {
        cerr << "Error, network with 2 inputs required." << endl;
        return 1;
    }
    if (net.outputs[0].size() != net.inputs[1].size()) {
        cerr << "Error, output must have the same size as 2nd input" << endl;
        return 1;
    }

    cout << "Predict input type: "  << (int)net.inputs[0].data_type() << endl;
    cout << "Predict output type: " << (int)net.outputs[0].data_type() << endl;
    
    // Init all input tensors with "1"
    for (Tensor& tensor : net.inputs) {
       *(half_float::half*)tensor.data() = 1.0f;
       //*(uint8_t*)tensor.data() = 1;
    }

    if (!copy && !swap) {
        // Use input buffer also for output so no copy nor swapping is needed.
        // Might not work if the output is computed directly from the input.
        cout << "Sharing input and output tensors." << endl;
        net.outputs[0].set_buffer(net.inputs[1].buffer());
    }

    // Run inference, given network binary graph and graph metadata
    for (int32_t i = 0; i < num_predict; i++) {
        // Do inference
        if (!net.predict()) {
            cerr << "Predict #" << i << ": failed" << endl;
            return 1;
        }
        cout << "Predict #" << i << ": " << *(half_float::half*)net.outputs[0].data() << endl;
        //cout << "Predict #" << i << ": " << (int)*(uint8_t*)net.outputs[0].data() << endl;
        
        if (copy) {
            // Copy result back to 2nd input (ok for small tensors)
            net.inputs[1].assign(net.outputs[0]);
        }
        if (swap) {
            // Swap output with 2nd input (ok for big tensors)
            Buffer* in_buffer2 = net.inputs[1].buffer();
            net.inputs[1].set_buffer(net.outputs[0].buffer());
            net.outputs[0].set_buffer(in_buffer2);
        }
    }

    return 0;
}
