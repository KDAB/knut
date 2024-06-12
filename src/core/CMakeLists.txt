# This file is part of Knut.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group
# company <info@kdab.com>
#
# SPDX-License-Identifier: GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(knut-core LANGUAGES CXX)

configure_file(version.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/version.cpp)

set(PROJECT_SOURCES
    astnode.h
    astnode.cpp
    classsymbol.h
    classsymbol.cpp
    codedocument.h
    codedocument.cpp
    codedocument_p.h
    codedocument_p.cpp
    conversionprogressdialog.h
    conversionprogressdialog.cpp
    conversionprogressdialog.ui
    cppdocument.h
    cppdocument.cpp
    cppdocument_p.h
    cppdocument_p.cpp
    functionsymbol.h
    functionsymbol.cpp
    dataexchange.h
    dataexchange.cpp
    dir.h
    dir.cpp
    document.h
    document.cpp
    file.h
    file.cpp
    fileinfo.h
    fileinfo.cpp
    imagedocument.h
    imagedocument.cpp
    knutcore.h
    knutcore.cpp
    logger.h
    logger.cpp
    mark_p.h
    mark.h
    mark.cpp
    message.h
    message.cpp
    messagemap.h
    messagemap.cpp
    project.h
    project.cpp
    project_p.h
    qdirvaluetype.h
    qdirvaluetype.cpp
    qfileinfovaluetype.h
    qfileinfovaluetype.cpp
    rangemark_p.h
    querymatch.h
    querymatch.cpp
    rangemark.h
    rangemark.cpp
    rcdocument.h
    rcdocument.cpp
    scriptdialogitem.h
    scriptdialogitem.cpp
    scriptdialogitem_p.h
    scriptdialogitem_p.cpp
    scriptitem.h
    scriptitem.cpp
    scriptmanager.h
    scriptmanager.cpp
    scriptmodel.h
    scriptmodel.cpp
    scriptrunner.h
    scriptrunner.cpp
    settings.h
    settings.cpp
    slintdocument.h
    slintdocument.cpp
    symbol.h
    symbol.cpp
    testutil.h
    testutil.cpp
    textdocument.h
    textdocument.cpp
    textdocument_p.h
    texteditor.h
    texteditor.cpp
    textlocation.h
    textlocation.cpp
    textrange.h
    textrange.cpp
    qtuidocument.h
    qtuidocument.cpp
    qttsdocument.h
    qttsdocument.cpp
    qmldocument.h
    qmldocument.cpp
    userdialog.h
    userdialog.cpp
    utils.h
    utils.cpp
    version.h
    ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
    core.qrc)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC nlohmann_json::nlohmann_json
         pugixml::pugixml
         kdalgorithms
         Qt${QT_VERSION_MAJOR}::Core
         Qt${QT_VERSION_MAJOR}::CorePrivate
         Qt${QT_VERSION_MAJOR}::Qml
         Qt${QT_VERSION_MAJOR}::QmlPrivate
         Qt${QT_VERSION_MAJOR}::Quick
         Qt${QT_VERSION_MAJOR}::Widgets
         Qt${QT_VERSION_MAJOR}::WidgetsPrivate
         Qt${QT_VERSION_MAJOR}::UiTools
         knut-lsp
         knut-treesitter
         knut-rccore
         knut-utils)
target_include_directories(${PROJECT_NAME}
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
