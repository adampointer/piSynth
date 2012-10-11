{
    "targets": [
        {
            "target_name": "oscillator",
            "sources": ["src/main.cpp"],
            "libraries": ["-lpthread", "-lm", "-lasound"],
            "cflags": ["-fpermissive"]
        }
    ]
}
