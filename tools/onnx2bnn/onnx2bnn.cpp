// Copyright 2019 JD.com Inc. JD AI

#include <fstream>
#include <map>
#include <numeric>
#include <string>

#include <common/StrKeyMap.h>
#include <common/argh.h>
#include <glog/logging.h>
#include "NodeAttrHelper.h"
#include "OnnxConverter.h"
#include "common/log_helper.h"

using std::string;
using std::vector;

void usage(const std::string &filename) {
    std::cout << "Usage:" << std::endl;
    std::cout << "  " << filename
              << " onnx_model output_filename [ --strict | --moderate | "
                 "--aggressive ]"
              << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout
        << "  --aggressive    The default optimization level. In this level, "
           "onnx2bnn will mark all convolutions with binary (+1/-1) weights as "
           "binary convolutions. It is for the existing BNN models, which may "
           "not use the correct padding value. Note: The output of the "
           "generated dabnn model is different from that of the ONNX model "
           "since the padding value is 0 instead of -1."
        << std::endl;
    std::cout << "  --moderate      This level is for our \"standard\" "
                 "implementation -- A Conv operator with binary weight and "
                 "following a -1 Pad operator."
              << std::endl;
    std::cout
        << "  --strict        In this level, onnx2bnn only recognizes the "
           "following natural and correct \"pattern\" of binary convolutions: "
           "A Conv operator, whose input is got from a Sign op and a Pad op "
           "(the order doesn't matter), and weight is got from a Sign op."
        << std::endl;
    std::cout << std::endl;
    std::cout << "Example:" << std::endl;
    std::cout << "  " << filename
              << " model.onnx model.dab (The optimization leval will be "
                 "\"aggressive\")"
              << std::endl;
    std::cout << "  " << filename
              << " model.onnx model.dab --strict (The optimization "
                 "level will be \"strict\")"
              << std::endl;
}

int main(int argc, char **argv) {
    argh::parser cmdl;
    cmdl.parse(argc, argv);
    google::InitGoogleLogging(cmdl[0].c_str());
    FLAGS_alsologtostderr = true;
    if (!cmdl(2)) {
        usage(cmdl[0]);
        return -1;
    }
    for (const auto flag : cmdl.flags()) {
        if (flag != "strict" && flag != "moderate" && flag != "aggressive") {
            std::cout << "Invalid flag: " << flag << std::endl;
            usage(cmdl[0]);
            return -2;
        }
    }

    bnn::OnnxConverter::Level opt_level =
        bnn::OnnxConverter::Level::kAggressive;
    if (cmdl["strict"]) {
        opt_level = bnn::OnnxConverter::Level::kStrict;
    } else if (cmdl["moderate"]) {
        opt_level = bnn::OnnxConverter::Level::kModerate;
    } else if (cmdl["aggressive"]) {
        opt_level = bnn::OnnxConverter::Level::kAggressive;
    }

    ONNX_NAMESPACE::ModelProto model_proto;
    {
        std::ifstream ifs(cmdl[1], std::ios::in | std::ios::binary);
        model_proto.ParseFromIstream(&ifs);
        ifs.close();
    }

    bnn::OnnxConverter converter;
    converter.Convert(model_proto, cmdl[2], opt_level);

    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
