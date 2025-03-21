// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.
///
/// Synap command line application for Object Detection with the model split in two parts.
///

#include "synap/arg_parser.hpp"
#include "synap/input_data.hpp"
#include "synap/preprocessor.hpp"
#include "synap/network.hpp"
#include "synap/detector.hpp"
#include "synap/timer.hpp"
#include "synap/label_info.hpp"
#include "synap/file_utils.hpp"
#include <iomanip>
#include <iostream>


using namespace std;
using namespace synaptics::synap;


int main(int argc, char* argv[])
{
    ArgParser args(argc, argv, "Object detection on image files", "[options] file(s)");
    string model = args.get("-m", "<file> Model file (.synap or legacy .nb)");
    string nb = args.get("--nb", "<file> Deprecated, same as -m");
    string meta = args.get("--meta", "<file> json meta file for legacy .nb models");
    string model2 = args.get("-m2", "<file> 2nd model file (.synap or legacy .nb)");
    string nb2 = args.get("--nb2", "<file> Deprecated, same as -m2");
    string meta2 = args.get("--meta2", "<file> 2nd json meta file for legacy .nb models");
    bool partial_buffer = args.has("--partial", "Use partial buffer sharing");
    float score_threshold = stof(args.get("--score-threshold", "<thr> Min confidence", "0.5"));
    int n_max = stoi(args.get("--max-detections", "<n> Max number of detections [0: all]", "0"));
    bool nms = !args.has("--disable-nms", "Disable Non-Max-Suppression algorithm");
    float iou_threshold = stof(args.get("--iou-threshold", "<thr> IOU threashold for NMS", "0.5"));
    bool iou_with_min = args.has("--iou-with-min", "Use min area instead of union to compute IOU");
    args.check_help("--help", "Show help");
    validate_model_arg(model, nb, meta);
    validate_model_arg(model2, nb2, meta2);

    Preprocessor preprocessor;
    Network network2;
    Network network1;
    Detector detector(score_threshold, n_max, nms, iou_threshold, iou_with_min);
    LabelInfo info(file_find_up("info.json", filename_path(model2)));
    cout << "Loading network 1: " << model << endl;
    if (!network1.load_model(model, meta)) {
        cerr << "Failed to load model" << endl;
        return 1;
    }
    cout << "Loading network 2: " << model2 << endl;
    if (!network2.load_model(model2, meta2)) {
        cerr << "Failed to load model2" << endl;
        return 1;
    }
    if (!detector.init(network2.outputs)) {
        cerr << "Failed to initialize detector" << endl;
        return 1;
    }

    // Check that the two networks the same number of in/out tensors
    if (network1.outputs.size() != network2.inputs.size()) {
        cerr << "Tensor count mismatch: network1 outputs: " << network1.outputs.size() <<
                ", network2 inputs: " << network2.inputs.size() << endl;
        return 1;
    }
    // Check that the two networks have compatible in/out tensors
    for(auto i = 0; i < network2.inputs.size(); i++) {
        if (!partial_buffer && network1.outputs[i].shape() != network2.inputs[i].shape()) {
            cerr << "Tensor " << i << " mismatch: network1 output shape: " << network1.outputs[i].shape() <<
                    ", network2 input shape: " << network2.inputs[i].shape() << endl;
            return 1;
        }
    }

    string input_filename;
    while ((input_filename = args.get()) != "") {
        // Load input data
        cout << " " << endl;
        InputData image(input_filename);
        if (image.empty()) {
            cerr << "Error, unable to read data from file: " << input_filename << endl;
            return 1;
        }

        Timer t;
        Rect assigned_rect;
        if (!preprocessor.assign(network1.inputs, image, 0, &assigned_rect)) {
            cerr << "Error assigning input to tensor." << endl;
            return 1;
        }
        auto t0 = t.get();
        Dimensions dim = image.dimensions();
        cout << "Input image: " << input_filename << " (w = " << dim.w
             << ", h = " << dim.h << ", c = " << dim.c << ")" << endl;

        // Execute inference
        if (!network1.predict()) {
            cerr << "Inference failed" << endl;
            return 1;
        }
        for(auto i = 0; i < network2.inputs.size(); i++) {
            Tensor& out = network1.outputs[i];
            Tensor& in = network2.inputs[i];
            if (out.item_count() == in.item_count()) {
                in.assign(out.as_float(), out.item_count());
            }
            else {
                // Truncate or pad the output with 0s to match input tensor size
                vector<float> tmp{out.as_float(), out.as_float() + out.item_count()};
                tmp.resize(in.item_count());
                in.assign(tmp.data(), tmp.size());
            }
        }
        if (!network2.predict()) {
            cerr << "Inference using the second network failed." << endl;
            return 1;
        }
        auto t1 = t.get();

        // Postprocess network outputs
        Detector::Result result = detector.process(network2.outputs, assigned_rect);
        if (!result.success) {
            cout << "Detection failed" << endl;
            return 1;
        }
        auto t2 = t.get();
        cout << "Detection time: " << t
             << " (pre:" << t0 << ", inf:" << t1 - t0 << ", post:" << t2 - t1 << ")" << endl;

        if (output_redirected()) {
            // Generate output in json so that it can be easily parsed by other tools
            cout << to_json_str(result) << endl;
        }
        else {
            cout << "#   Score  Class   Position        Size  Description     Landmarks" << endl;
            int j = 0;
            for (const auto& item : result.items) {
                const auto& bb = item.bounding_box;
                cout << setw(3) << left << j++ << right << "  " << item.confidence << " " << setw(6)
                     << item.class_index << "  " << setw(4) << right << bb.origin.x << "," << setw(4)
                     << right << bb.origin.y << "   " << setw(4) << bb.size.x << "," << setw(4)
                     << right << bb.size.y <<  "  " << setw(16) << left << info.label(item.class_index);
                for (const auto lm: item.landmarks) {
                    cout << lm << " ";
                }
                cout << endl;
            }
        }
    }

    return 0;
}
