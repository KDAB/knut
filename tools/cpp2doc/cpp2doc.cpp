/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "data.h"
#include "docwriter.h"
#include "sourceparser.h"

int main(int argc, char *argv[])
{
    Q_UNUSED(argc)
    Q_UNUSED(argv)

    Data data;
    SourceParser parser(data);
    parser.loadMappingFile(KNUT_DOC_PATH "/API");
    parser.parseDirectory(KNUT_SOURCE_PATH "/core");
    parser.parseDirectory(KNUT_SOURCE_PATH "/rccore");

    DocWriter writer(data);
    writer.saveDocumentation(KNUT_DOC_PATH "/API");
}
