// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.
///
/// Synap command line application for Image Classification.
///

#include "synap/input_data.hpp"
#include "synap/preprocessor.hpp"
#include "synap/network.hpp"
#include "synap/classifier.hpp"
#include "synap/timer.hpp"
#include "synap/arg_parser.hpp"
#include "synap/label_info.hpp"
#include "synap/file_utils.hpp"

using namespace std;
using namespace synaptics::synap;


// MobileNet V2 is a classification model with input shape 224x224x3
#if __ANDROID__
#define BASE_PATH "/vendor/firmware/models/image_classification/imagenet/"
#else
#define BASE_PATH "/usr/share/synap/models/image_classification/imagenet/"
#endif

int main(int argc, char** argv)
{
    const string model_default_dir = BASE_PATH "model/mobilenet_v2_1.0_224_quant/";
    const string sample_default = BASE_PATH "sample/space_shuttle_224x224.jpg";
    ArgParser args(argc, argv, "Image classification on image files", "[options] file(s)");
    string model = args.get("-m", "<file> Model file (.synap or legacy .nb)");
    string nb = args.get("--nb", "<file> Deprecated, same as -m");
    string meta = args.get("--meta", "<file> json meta file for legacy .nb models");
    int top = stoi(args.get("--top", "<n> Number of classification results to show", "5"));
    string roi_str = args.get("--roi", "[x,y,h,w] Region of Interest");
    args.check_help("--help", "Show help");
    validate_model_arg(model, nb, meta, model_default_dir);

    Rect roi{};
    if (!roi_str.empty() && !from_string(roi, roi_str)) {
        cerr << "Error, invalid ROI rectangle: " << roi_str << endl;
        return 1;
    }

    // Get sample images (or use default image if none specified)
    vector<string> images;
    string input_filename;
    while ((input_filename = args.get()) != "") {
        images.push_back(input_filename);
    }
    if (images.empty()) {
        images.push_back(sample_default);
    }

    // Create the preprocessor, model, postprocessor and label description 
    Preprocessor preprocessor;
    Network network;
    Classifier classifier(top);
    LabelInfo info(file_find_up("info.json", filename_path(model)));
    cout << "Loading network: " << model << endl;
    if (!network.load_model(model, meta)) {
        cerr << "Failed to load model" << endl;
        return 1;
    }

    // Run inference on all input images
    preprocessor.set_roi(roi);
    for (const auto& image_name : images) {
        // Load input data
        cout << "Input image: " << image_name << endl;
        InputData input_image(image_name);
        if (input_image.empty()) {
            cerr << "Error, unable to read data from file: " << image_name << endl;
            return 1;
        }

        // Preprocess the input data and assign it to the network input tensor
        Timer t;
        if (!preprocessor.assign(network.inputs[0], input_image)) {
            cerr << "Error assigning input to tensor." << endl;
            return 1;
        }
        auto t0 = t.get();

        // Execute inference
        if (!network.predict()) {
            cerr << "Inference failed" << endl;
            return 1;
        }
        auto t1 = t.get();

        // Postprocess network outputs
        Classifier::Result result = classifier.process(network.outputs);
        if (!result.success) {
            cerr << "Classification failed." << endl;
            return 1;
        }
        auto t2 = t.get();
        cout << "Classification time: " << t
             << " (pre:" << t0 / 1e3 << ", inf:" << (t1 - t0) / 1e3 << ", post:" << (t2 - t1) / 1e3 << ")" << endl;

        // Print classification results
        if (output_redirected()) {
            // Generate output in json so that it can be easily parsed by other tools
            cout << to_json_str(result) << endl;
        }
        else {
            cout << "Class  Confidence  Description" << fixed << setprecision(4) << endl;
            for (auto item: result.items) {
                cout << setw(5) << item.class_index << setw(12) << item.confidence << "  "
                     << info.label(item.class_index) << endl;
            }
        }
    }

    return 0;
}
