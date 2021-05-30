#include "data.h"
#include "docwriter.h"
#include "sourceparser.h"

int main(int argc, char *argv[])
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    Data data;
    SourceParser parser(data);
    parser.parseDirectory(KNUT_SOURCE_PATH "/core");

    DocWriter writer(data);
    writer.saveDocumentation();
}
