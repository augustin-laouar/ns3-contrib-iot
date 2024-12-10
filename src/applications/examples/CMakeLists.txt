build_lib_example(
  NAME three-gpp-http-example
  SOURCE_FILES three-gpp-http-example.cc
  LIBRARIES_TO_LINK
    ${libapplications}
    ${libpoint-to-point}
    ${libinternet}
    ${libnetwork}
)

build_lib_example(
  NAME iot-passive-app-example
  SOURCE_FILES iot-passive-app-example.cc
  LIBRARIES_TO_LINK
    ${libapplications}
    ${libinternet}
    ${libnetwork}
    ${libwifi}
    ${libmobility}
)
