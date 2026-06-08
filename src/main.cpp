#include "MainWindow.h"
#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("VEA Djvu Reader");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("OpenDjVu");

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "DjVu file to open");
    parser.process(app);

    MainWindow w;
    w.show();

    const auto args = parser.positionalArguments();
    if (!args.isEmpty()) w.openFile(args.first());

    return app.exec();
}
