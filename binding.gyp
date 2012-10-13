{
    "targets": [
        {
            "target_name": "oscillator",
            "sources": [
                "src/main.cpp"
            ],
            "libraries": [
                "-lpthread",
                "-lm",
                "-lasound"
            ],
            "cflags": [
                "-fpermissive",
                "-march=armv6",
                "-mfpu=vfp",
                "-mfloat-abi=hard"
            ],
            "defines": [
                "ARCH='<(target_arch)'",
                "PLATFORM='<(OS)'"
            ],
            "conditions": [
                [
                    "OS=='linux'",
                    {
                        "cflags": [
                            "-fpermissive"
                        ],
                        "conditions": [
                            [
                                "target_arch=='arm'",
                                {
                                    "cflags": [
                                        "-fpermissive",
                                        "-march=armv6",
                                        "-mfpu=vfp",
                                        "-mfloat-abi=hard"
                                    ]
                                }
                            ]
                        ]
                    }
                ]
            ]
        }
    ]
}
