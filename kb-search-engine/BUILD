load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "kb-search-engine",
    srcs = glob(["src/**/*.cpp"], exclude=["entrypoint.cpp"]),
    hdrs = glob(["include/**/*.h"]),
    copts = ["-Ikb-search-engine/include"],
    # strip_include_prefix = "include",
    visibility = ["//visibility:public"],
    deps = [
        "@spdlog"
    ],
)

cc_binary(
    name = "kb-search-cli",
    srcs = glob(["src/**/*.cpp"]),
    copts = ["-Ikb-search-engine/include"],
    deps = ["//kb-search-engine:kb-search-engine"],
)