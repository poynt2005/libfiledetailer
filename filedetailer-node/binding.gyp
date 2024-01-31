{
    "targets": [
        {
            "target_name": "FileDetailer",
            "cflags!": ["-fno-exceptions"],
            "cflags_cc!": ["-fno-exceptions"],
            "sources": ["lib/native/Binding.cc"],
            'win_delay_load_hook': 'true',
            "include_dirs": [
                "<!@(node -p \"require('node-addon-api').include\")",
                "<!@(node -p \"require('./path').fdIncl\")",
            ],
            'defines': ['NAPI_DISABLE_CPP_EXCEPTIONS'],
            'libraries': [
                "<!@(node -p \"require('./path.js').fdLib\")",
            ],
            'msvs_settings': {
                'VCCLCompilerTool': {
                    'AdditionalOptions': ['-std:c++20', ]
                },
            }
        }
    ]
}
