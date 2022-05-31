#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTextStream>

#include "fileio/fileio.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    // parse command line
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    // user name (-u, --user)
    QCommandLineOption inputFilePar(QStringList() << "f" << "file",
                                QCoreApplication::translate("main", "Username"),
                                QCoreApplication::translate("main", "username"));
    parser.addOption(inputFilePar);

    // Process the actual command line arguments given by the user
    parser.process(a.arguments());

    QString inputFileName = parser.value(inputFilePar);

    QFile inputFile(inputFileName);
    if (!inputFile.open(QIODevice::ReadOnly)) {
        fprintf(stdout, "Error open file");
        fflush(stdout);
        return 0;
    }

    QList<int> marksVector;
    QDataStream in(&inputFile);
    // Convert file --------
    {
        QStringList seriesNames = {
            "adc_value"
        };

        FileIO::FileWriter _writer(seriesNames, 0, 0.174);
        QString filename = QString("%1_convert")
            .arg(inputFileName);
        _writer.openFile(filename);

        while (!in.atEnd()) {
            QVector<qreal> series;
            uint16_t val;
            in.readRawData((char *)&val, 2);
            series << val;
            _writer.writePoint(series);
        }

        _writer.closeFile();
    }

    fprintf(stdout, "End");
    fflush(stdout);
    //----------------------

    return a.exec();
}
