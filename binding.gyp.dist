{
  "targets": [
    {
      "target_name": "vibrancy",
      "conditions":[
        ["OS=='win'", {
          "sources": [
            "native/vibrancy.cc",
            "native/custom-blur.cc"
          ]
        }]
      ],
      "cflags": [
        "-O3"
      ],
      "libraries": [
        "windowsapp.lib",
        "d2d1.lib",
        "dxguid.lib"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "defines": [
        "NAPI_DISABLE_CPP_EXCEPTIONS"
      ]
    }
  ]
}
