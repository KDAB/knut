file(GLOB QML_FILES *.qml)
add_custom_target(test_data SOURCES ${QML_FILES})
