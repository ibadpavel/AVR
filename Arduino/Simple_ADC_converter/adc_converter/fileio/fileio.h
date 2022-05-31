#ifndef FILEIO_H
#define FILEIO_H

#include "rangetemplate.h"
#include <QString>
#include <QFile>
#include <QDataStream>
#include <QSharedPointer>
#include <QObject>

namespace FileIO {

using QFilePointer = QSharedPointer<QFile>;
using QStreamPointer = QSharedPointer<QDataStream>;

class ScaleInfo
{
public:
    void clear();
    void init(qreal xStep);

    // getters
    qint64 base() const;
    qint64 firstIterationBase() const;
    qint64 pointsCombinedCount(int scalingIndex) const;
    int scalingIndex(qreal pixelWidth) const;
    qreal xStep(int scalingIndex) const;

    // static
    static qint64 defaultScalingBase();
    static void setScalingBase(qint64 value);

protected:
    void setXStep(qreal value);

private:
    static qint64 m_scalingBase;
    qreal  m_xStep = m_defaultXStep;

    static const int    m_cachePointSize;
    static const qint64 m_defaultScalingBase;
    static const qreal  m_defaultXStep;
};


class CacheInfo
{
    struct Block {
        qint64 byteOffset = 0;
        qint64 xCount = 0;
    };

public:
    void init(const QString &fileName, qint64 pointsCount, const ScaleInfo &scaleInfo);
    QString toString() const;

    // getters
    static int cachePointSize();
    QString fileName(int seriesIndex) const;
    qint64 fileSize() const;
    int maxValidScalingIndex() const;
    qint64 byteOffset(int scalingIndex, qint64 xIndex = 0);
    qint64 yCount(int scalingIndex) const;
    qint64 xCount(int scalingIndex) const;
    qint64 xMaxIndex(int scalingIndex) const;

protected:
    void addBlock(qint64 xCount);
    bool isValidScalingIndex(int scalingIndex) const;
    void setFileName(const QString &fileName);

private:
    QVector<Block> m_blocks;
    QString m_fileNameWithoutExtension;

    static const int m_cachePointSize;
    static const int m_cachePointByteSize;
};


class AbstractFileIO/*: public QObject*/
{
//    Q_OBJECT

protected:
    // RLE = run-length encoding
    enum FileFormat {
        General    = 0,
        RLEzip     = 1,
        OldGeneral = 2,
        OldRLEzip  = 3,
        Unknown    = 4
    };

    struct Metadata {
        FileFormat  fileFormat = FileFormat::Unknown;
        QStringList seriesNameList;
        qreal       xMin = 0;
        qreal       xStep = 1;
    };

    // legacy
    struct FileHeader {
        char signature[8];          // code header, indicating the binary file filling signal
        char undefinedChar1A[100];  // name of the element from which the signal is removed
        char undefinedChar2A[100];  // the name of the output signal from which is removed
        int undefinedInt1;          // Element ID is removed from the signal
        int undefinedInt2;          // ID output signal from which is removed
        int undefinedInt3;          // signal type 0 - integer, 1 - real, 2 - complex
        int undefinedInt4;          // flag that the signal was taken uniformly with a given period
        double xMin;                // the start time point (used to write a regular situation)
        double xStep;               // the sampling period of the signal
        int pointsCount;            // many points
        int seriesNameCount;        // the number of elements in the structure of a signal which contains a data structure
        int undefinedInt5A[50];
        // after the structure in the file there is a list of signal names with a length of 40 characters
    };

public:
    AbstractFileIO();
    AbstractFileIO(const AbstractFileIO &) = delete;
    virtual ~AbstractFileIO();

    virtual bool openFile(const QString &fileName) = 0;

    // getters
    QString errorString() const;
    QString fileName() const;
    bool hasError() const;

protected:
    friend QDataStream &operator>> (QDataStream &stream, Metadata &data);
    friend QDataStream &operator<< (QDataStream &stream, const Metadata &data);

    friend QDataStream &operator<< (QDataStream &out, const FileHeader &data);
    friend QDataStream &operator>> (QDataStream &in, FileHeader &data);

    static const QString &defaultSeriesName();

    static QString fileFormatToString(const FileFormat &format);
    static FileFormat stringToFileFormat(const QString &format);

    static FileFormat readFileSignature(QDataStream &stream);
    static FileFormat readFileSignature(const QString &fileName);

protected:
    QFilePointer   m_file;
    QStreamPointer m_dataStream;

private:
    static const QString m_defaultSeriesName;
    static const QStringList m_fileFormatStringList;
};

// readers

class AbstractFileReader: public QObject, public AbstractFileIO
{
    Q_OBJECT

public:
    explicit AbstractFileReader(QObject *parent = nullptr);
    virtual ~AbstractFileReader();

    virtual bool openFile(const QString &fileName);
    bool createCache(QVector<bool> selectedSeries, bool forceCacheUpdate = false); // protected
    bool deleteCache();
    bool deleteCache(int seriesIndex);
    bool isEqual(qreal lhs, qreal rhs) const;
    bool isLess(qreal lhs, qreal rhs) const;
    bool readSeries(int seriesIndex, qreal xFirst, qreal xLast, int scalingIndex, QVector<QPointF> &points);
    bool readSeries(int seriesIndex, RangeF xValue, int scalingIndex, QVector<QPointF> &points);
    bool readSeries(int seriesIndex, qint64 xFirst, qint64 xLast, int scalingIndex, QVector<QPointF> &points);
    bool readSeries(int seriesIndex, Range xIndex, int scalingIndex, QVector<QPointF> &points);
    bool readSeriesY(int seriesIndex, const Range &xIndex, QVector<qreal> &y);
    Range xIndexByValue(RangeF xValue, int scalingIndex = 0) const;

    // getters
    QVector<bool> cacheFilesExist() const;
    qint64 pointsCount() const;
    const ScaleInfo *scaleInfo() const;
    int seriesCount() const;
    const QString &seriesName(int index) const;
    const QStringList &seriesNameList() const;
    const RangeF &xLimit() const;
    RangeF yLimit(int seriesIndex);
    qreal xMax() const;
    qreal xMin() const;
    qreal xStep() const;

signals:
    void cacheCreationProgress(int value);

protected:
    // cache
    bool cacheExists() const;
    bool cacheExists(int seriesIndex) const;
    bool createCache(bool forced = false);
    bool createCacheForOneSeries(int seriesIndex, QFilePointer outFile, bool forced = false);
    virtual bool createCacheForOneSeriesFirstIteration(int seriesIndex, QFilePointer outFile) = 0;
    bool createCacheForOneSeriesNextIterations(int seriesIndex, QFilePointer outFile);
    bool deleteCache() const;
    bool deleteCache(int seriesIndex) const;
    bool readCache(int seriesIndex, Range xIndex, int scalingIndex, QVector<qreal> &values);

    void initInfo();
    virtual bool readFile(int seriesIndex, Range xIndex, QVector<QPointF> &points) = 0;
    virtual bool readFile(int seriesIndex, Range xIndex, QVector<qreal> &y) = 0;
    void setFilePositionToPoint(qint64 pointIndex);

    // validation
    bool isValidSeriesIndex(int seriesIndex) const;
    bool isValidXIndex(const Range &xIndex, int scalingIndex) const;
    bool isValidXValue(RangeF xValue) const;

    qreal xValueByIndex(qint64 index) const;
    qint64 xIndexByValue(qreal value) const;

protected:
    qint64 m_sizeOfMetadata;
    qint64 m_sizeOfPoint;

    qint64 m_pointsCount;
    int    m_seriesCount;

    RangeF m_xLimit;
    qint64 m_xMaxIndex;
    qreal  m_xStep;

    CacheInfo m_cacheInfo;
    ScaleInfo m_scaleInfo;

    QStringList m_seriesNameList;
    QVector<bool> m_selectedSeries;

private:
    qreal m_epsilon;
};


class FileReader: public AbstractFileReader
{
    Q_OBJECT

protected:
    struct ReadBuffer {
        QVector<qreal> yValues;
    };

public:
    explicit FileReader(QObject *parent = nullptr);
    virtual ~FileReader();
    virtual bool openFile(const QString &fileName) override;

protected:
    virtual bool createCacheForOneSeriesFirstIteration(int seriesIndex, QFilePointer outFile) override;
    virtual bool readFile(int seriesIndex, Range xIndex, QVector<QPointF> &points) override;
    virtual bool readFile(int seriesIndex, Range xIndex, QVector<qreal> &y) override;

    void readMinMaxSeriesAndWriteToStream(int seriesIndex, ReadBuffer &buffer,
                                          QStreamPointer &inStream, QStreamPointer &outStream);
    void readY(int seriesIndex, qreal &y);
    void readY(int seriesIndex, qreal &y, QStreamPointer &inStream);
};


class ZipFileReader: public AbstractFileReader
{
    Q_OBJECT

protected:
    struct ZipReadBuffer {
        qint64 xIndex, xIndexRight;
        qreal yValueLeft, yValueRight;
        QVector<qreal> yValues;
    };

public:
    explicit ZipFileReader(QObject *parent = nullptr);
    virtual ~ZipFileReader();
    virtual bool openFile(const QString &fileName) override;

protected:
    virtual bool createCacheForOneSeriesFirstIteration(int seriesIndex, QFilePointer outFile) override;
    virtual bool readFile(int seriesIndex, Range xIndex, QVector<QPointF> &points) override;
    virtual bool readFile(int seriesIndex, Range xIndex, QVector<qreal> &y) override;

    void readMinMaxSeriesAndWriteToStream(int seriesIndex, ZipReadBuffer &buffer,
                                          QStreamPointer &inStream, QStreamPointer &outStream);
    void readXY(int seriesIndex, qint64 &x, qreal &y);
    void readXY(int seriesIndex, qint64 &x, qreal &y, QStreamPointer &inStream);

    void readXIndexByZipIndex(qint64 xZipIndex, qint64 &xIndex);
    qint64 xZipIndexByIndex(qint64 xIndex);

protected:
    qint64 m_zipPointsCount;
};


class OldFileReader: public FileReader
{
    Q_OBJECT

public:
    explicit OldFileReader(QObject *parent = nullptr);
    virtual ~OldFileReader();
    virtual bool openFile(const QString &fileName) override;

protected:
    virtual bool createCacheForOneSeriesFirstIteration(int seriesIndex, QFilePointer outFile) override;
    virtual bool readFile(int seriesIndex, Range xIndex, QVector<QPointF> &points) override;
    virtual bool readFile(int seriesIndex, Range xIndex, QVector<qreal> &y) override;

    void readMinMaxSeriesAndWriteToStream(int seriesIndex, ReadBuffer &buffer,
                                          QStreamPointer &inStream, QStreamPointer &outStream);
    void readY(int seriesIndex, qreal &y);
    void readY(int seriesIndex, qreal &y, QStreamPointer &inStream);
};


class OldZipFileReader: public ZipFileReader
{
    Q_OBJECT

public:
    explicit OldZipFileReader(QObject *parent = nullptr);
    virtual ~OldZipFileReader();
    virtual bool openFile(const QString &fileName) override;

protected:
    virtual bool createCacheForOneSeriesFirstIteration(int seriesIndex, QFilePointer outFile) override;
    virtual bool readFile(int seriesIndex, Range xIndex, QVector<QPointF> &points) override;
    virtual bool readFile(int seriesIndex, Range xIndex, QVector<qreal> &y) override;

    void readMinMaxSeriesAndWriteToStream(int seriesIndex, ZipReadBuffer &buffer,
                                          QStreamPointer &inStream, QStreamPointer &outStream);
    void readXY(int seriesIndex, qint64 &x, qreal &y);
    void readXY(int seriesIndex, qint64 &x, qreal &y, QStreamPointer &inStream);

    void readXIndexByZipIndex(qint64 xZipIndex, qint64 &xIndex);
    qint64 xZipIndexByIndex(qint64 xIndex);
};


// writers

class AbstractFileWriter: public AbstractFileIO
{
public:
    AbstractFileWriter(QStringList seriesNames, qreal xMin = 0, qreal xStep = 1);
    virtual ~AbstractFileWriter();

    void closeFile();
    static const QString &fileExtension();
    bool openFile(const QString &fileName);
    virtual void writePoint(const QVector<qreal> &point) = 0;

protected:
    static const QString m_fileExtension;

protected:
    Metadata m_metadata;
    int m_seriesCount;
};


class FileWriter: public AbstractFileWriter
{
public:
    FileWriter(QStringList seriesNames, qreal xMin = 0, qreal xStep = 1);
    virtual ~FileWriter();

    virtual void writePoint(const QVector<qreal> &point) override;
};


class ZipFileWriter: public AbstractFileWriter
{
public:
    ZipFileWriter(QStringList seriesNames, qreal xMin = 0, qreal xStep = 1);
    virtual ~ZipFileWriter();

    virtual void writePoint(const QVector<qreal> &point) override;

private:
    qint64 m_currentXIndex;
    QVector<qreal> m_lastYValue;
    bool m_lastYChanged;
};

} // namespace

#endif // FILEIO_H
