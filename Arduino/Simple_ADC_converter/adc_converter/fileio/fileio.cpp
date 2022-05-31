#include "fileio.h"

#include <QtMath>
#include <QDateTime>
#include <QDebug> // del
#include <QElapsedTimer>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <vector>
#include <memory>

namespace FileIO {

// additional function
qint64 ceilDivision(qint64 dividend, qint64 divisor)
{
    return (dividend / divisor) + (dividend % divisor != 0); // = ceil for integer
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// ScaleInfo ////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

const int    ScaleInfo::m_cachePointSize = 2; // (x, yMin), (x, yMax);
const qint64 ScaleInfo::m_defaultScalingBase = 2;
const qreal  ScaleInfo::m_defaultXStep = 1;
qint64 ScaleInfo::m_scalingBase = m_defaultScalingBase;

/// Reset inner variables
void ScaleInfo::clear()
{
    m_scalingBase = m_defaultScalingBase;
    m_xStep = m_defaultXStep;
}


/// Initialization.
void ScaleInfo::init(qreal xStep)
{
    setXStep(xStep);
}

/// How many times the number of x values changes between iterations.
qint64 ScaleInfo::base() const
{
    return m_scalingBase;
}

/// The first iteration is a little different because cache contains
/// pairs of points with the same x value, i.e. (x, yMin), (x, yMax).
qint64 ScaleInfo::firstIterationBase() const
{
    return m_scalingBase * m_cachePointSize;
}

/// The number of origin points combined into one pair with the same x value.
qint64 ScaleInfo::pointsCombinedCount(int scalingIndex) const
{
    if (scalingIndex < 1) {
        return 1;
    }

    return m_cachePointSize * qPow(m_scalingBase, scalingIndex); // todo: store this and do not calculate?
}

/// The scaling index corresponding to the specified pixel width.
int ScaleInfo::scalingIndex(qreal pixelWidth) const
{
    return qFloor(qLn(pixelWidth / (m_xStep * m_cachePointSize)) / qLn(m_scalingBase));
}

/// The step of x values at a given scale.
qreal ScaleInfo::xStep(int scalingIndex) const
{
    return m_xStep * pointsCombinedCount(scalingIndex);
}

qint64 ScaleInfo::defaultScalingBase()
{
    return m_defaultScalingBase;
}

void ScaleInfo::setScalingBase(qint64 value)
{
    if (value <= 1) {
        m_scalingBase = m_defaultScalingBase;
    } else {
        m_scalingBase = value;
    }
}

void ScaleInfo::setXStep(qreal value)
{
    if (value <= 0) {
        m_xStep = m_defaultXStep;
    } else {
        m_xStep = value;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// CacheInfo ////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

const int CacheInfo::m_cachePointSize = 2;
const int CacheInfo::m_cachePointByteSize = CacheInfo::m_cachePointSize * sizeof(qreal);

void CacheInfo::init(const QString &fileName, qint64 pointsCount, const ScaleInfo &scaleInfo)
{
    setFileName(fileName);
    addBlock(pointsCount); // file points count

    pointsCount = ceilDivision(pointsCount, scaleInfo.firstIterationBase());
    addBlock(pointsCount);

    while (pointsCount > 1) {
        pointsCount = ceilDivision(pointsCount, scaleInfo.base());
        addBlock(pointsCount);
    }
}

QString CacheInfo::toString() const
{
    QString result;

    for (const auto &item : m_blocks) {
        result.append(QString("offset = %1, xCount = %2\n").arg(item.byteOffset).arg(item.xCount));
    }

    return result;
}

int CacheInfo::cachePointSize()
{
    return m_cachePointSize;
}

QString CacheInfo::fileName(int seriesIndex) const
{
    return m_fileNameWithoutExtension + QString("_%1.cache").arg(seriesIndex);
}

qint64 CacheInfo::fileSize() const
{
    return m_blocks.last().byteOffset + m_blocks.last().xCount * m_cachePointByteSize;
}

int CacheInfo::maxValidScalingIndex() const
{
    return m_blocks.size() - 1;
}

qint64 CacheInfo::byteOffset(int scalingIndex, qint64 xIndex)
{
    if (isValidScalingIndex(scalingIndex)) {
        return m_blocks.at(scalingIndex).byteOffset + xIndex * m_cachePointByteSize;
    } else {
        return 0;
    }
}

qint64 CacheInfo::yCount(int scalingIndex) const
{
    if (isValidScalingIndex(scalingIndex)) {
        return m_blocks.at(scalingIndex).xCount * m_cachePointSize;
    } else {
        return 0;
    }
}

qint64 CacheInfo::xCount(int scalingIndex) const
{
    if (isValidScalingIndex(scalingIndex)) {
        return m_blocks.at(scalingIndex).xCount;
    } else {
        return 0;
    }
}

qint64 CacheInfo::xMaxIndex(int scalingIndex) const
{
    return xCount(scalingIndex) - 1;
}

void CacheInfo::addBlock(qint64 xCount)
{
    Block newBlock;
    newBlock.xCount = xCount;

    if (m_blocks.size() > 1) {
        Block &lastBlock = m_blocks.last();
        newBlock.byteOffset = lastBlock.byteOffset + lastBlock.xCount * m_cachePointByteSize;
    }

    m_blocks.append(newBlock);
}

bool CacheInfo::isValidScalingIndex(int scalingIndex) const
{
    return (0 <= scalingIndex) && (scalingIndex < m_blocks.size());
}

void CacheInfo::setFileName(const QString &fileName)
{
    m_fileNameWithoutExtension = fileName.left(fileName.lastIndexOf("."));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// AbstractFileIO //////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

const QString AbstractFileIO::m_defaultSeriesName = "Untitled";

const QStringList AbstractFileIO::m_fileFormatStringList = {
    "General",
    "RLE_zip",
    "Digital",
    "Digizip",
    "Unknown"
};

AbstractFileIO::AbstractFileIO()
    : m_file(new QFile())
    , m_dataStream(new QDataStream(m_file.get()))
{
    m_dataStream->setVersion(QDataStream::Qt_5_12);
}

AbstractFileIO::~AbstractFileIO()
{

}

QString AbstractFileIO::errorString() const
{
    if (hasError()) {
        return QString("File status: %1, DataStream status: %2, message: %3")
               .arg(m_file->error()).arg(m_dataStream->status()).arg(m_file->errorString());
    }

    return QString("No errors.");
}

QString AbstractFileIO::fileName() const
{
    if (m_file.isNull()) {
        return QString();
    } else {
        return m_file->fileName();
    }
    return m_file->fileName();
}

bool AbstractFileIO::hasError() const
{
    return (m_file->error() != QFileDevice::NoError) || (m_dataStream->status() != QDataStream::Ok);
}

QDataStream &operator>> (QDataStream &stream, AbstractFileIO::Metadata &data)
{
    data.fileFormat = AbstractFileIO::readFileSignature(stream);

    if (data.fileFormat == AbstractFileIO::FileFormat::General
            || data.fileFormat == AbstractFileIO::FileFormat::RLEzip) {
        stream >> data.seriesNameList >> data.xMin >> data.xStep;
    } else if (data.fileFormat == AbstractFileIO::FileFormat::OldGeneral
               || data.fileFormat == AbstractFileIO::FileFormat::OldRLEzip) {
        stream.device()->seek(0);
        stream.setByteOrder(QDataStream::LittleEndian);

        AbstractFileIO::FileHeader header;
        stream >> header;

        int seriesNameMaxLength = 40; // legacy const value
        char seriesName[40];

        if (header.seriesNameCount > 0) {
            for (int i = 0; i < header.seriesNameCount; ++i) {
                stream.readRawData(seriesName, seriesNameMaxLength);
                data.seriesNameList.append(QString::fromLatin1(seriesName, seriesNameMaxLength));
            }
        } else { // one series without name
            data.seriesNameList.append(AbstractFileIO::defaultSeriesName());
        }

        data.xMin = header.xMin;
        if (data.fileFormat == AbstractFileIO::FileFormat::OldRLEzip) {
            data.xStep = header.xStep;
        } else { // todo: no points case
            if (stream.device()->bytesAvailable() < (2 + data.seriesNameList.size()) * sizeof(qreal)) {
                data.xStep = 1;
            } else { // at least two x values ​​in the file
                int sizeOfMetadata = stream.device()->pos();
                qreal x1;
                stream.skipRawData((1 + data.seriesNameList.size()) * sizeof(qreal));
                stream >> x1;
                data.xStep = x1 - data.xMin;
                stream.device()->seek(sizeOfMetadata);
            }
        }

        stream.setByteOrder(QDataStream::BigEndian);
    }

    return stream;
}

QDataStream &operator<< (QDataStream &stream, const AbstractFileIO::Metadata &data)
{
    QString signature = AbstractFileWriter::fileFormatToString(data.fileFormat);
    stream.writeRawData(signature.toLatin1().data(), signature.size());

    stream << data.seriesNameList << data.xMin << data.xStep;

    return stream;
}

QDataStream &operator<< (QDataStream &out, const AbstractFileIO::FileHeader &data)
{
    out.writeRawData(reinterpret_cast<const char *>(&data), sizeof(data));

    return out;
}

QDataStream &operator>> (QDataStream &in, AbstractFileIO::FileHeader &data)
{
    in.readRawData(reinterpret_cast<char *>(&data), sizeof(data));

    return in;
}

const QString &AbstractFileIO::defaultSeriesName()
{
    return m_defaultSeriesName;
}

QString AbstractFileIO::fileFormatToString(const FileFormat &format)
{
    int index = static_cast<int>(format);

    if ((index < 0) || (index >= m_fileFormatStringList.size())) {
        return m_fileFormatStringList.last();
    } else {
        return m_fileFormatStringList.at(index);
    }
}

AbstractFileIO::FileFormat AbstractFileIO::stringToFileFormat(const QString &format)
{
    int index = m_fileFormatStringList.indexOf(format);

    if (index == -1) {
        return FileFormat::Unknown;
    } else {
        return static_cast<FileFormat>(index);
    }
}

AbstractFileIO::FileFormat AbstractFileIO::readFileSignature(QDataStream &stream)
{
    int signatureSize = 40;//AbstractFileIO::m_fileFormatStringList.at(0).size();
    char signature[40];
    stream.readRawData(signature, signatureSize);

    return AbstractFileIO::stringToFileFormat(QString::fromLatin1(signature, signatureSize));
}

AbstractFileIO::FileFormat AbstractFileIO::readFileSignature(const QString &fileName)
{
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);

    int signatureSize = 40;//AbstractFileIO::m_fileFormatStringList.at(0).size();
    char signature[40];
    file.read(signature, signatureSize);

    return AbstractFileIO::stringToFileFormat(QString::fromLatin1(signature, signatureSize));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////// AbstractFileReader ////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

AbstractFileReader::AbstractFileReader(QObject *parent)
    : QObject(parent)
{

}

AbstractFileReader::~AbstractFileReader()
{

}

bool AbstractFileReader::openFile(const QString &fileName)
{
    // todo: add clear
    Metadata metadata;

    m_file->setFileName(fileName);
    m_file->open(QIODevice::ReadOnly);

    *m_dataStream >> metadata;
    m_sizeOfMetadata = m_file->pos();

    m_seriesNameList = metadata.seriesNameList;
    m_seriesCount = m_seriesNameList.size();

    m_xLimit.setMin(metadata.xMin);
    m_xStep = metadata.xStep;

    m_epsilon = metadata.xStep * std::numeric_limits<qreal>::epsilon();

    // uninitialized variables
    // m_sizeOfPoint, m_pointsCount, m_xLimit.max()
    // m_cacheInfo, m_scaleInfo, m_selectedSeries -> init()

    return !hasError();
}

bool AbstractFileReader::createCache(QVector<bool> selectedSeries, bool forceCacheUpdate)
{
    m_selectedSeries = selectedSeries;
    createCache(forceCacheUpdate);

    return !hasError();
}

bool AbstractFileReader::deleteCache()
{
    bool result = true;

    for (int i = 0; i < m_seriesCount; ++i) {
        if (!deleteCache(i)) {
            result = false;
        }
    }

    return result;
}

bool AbstractFileReader::deleteCache(int seriesIndex)
{
    QString fileName = m_cacheInfo.fileName(seriesIndex);

    if (QFileInfo::exists(fileName)) {
        return QFile::remove(fileName);
    }

    return false;
}

bool AbstractFileReader::isEqual(qreal lhs, qreal rhs) const
{
    return std::fabs(rhs - lhs) < m_epsilon;
}

bool AbstractFileReader::isLess(qreal lhs, qreal rhs) const
{
    if (isEqual(lhs, rhs)) {
        return false;
    } else {
        return lhs < rhs;
    }
}

bool AbstractFileReader::readSeries(int seriesIndex, qreal xFirst, qreal xLast,
                                    int scalingIndex, QVector<QPointF> &points)
{
    return readSeries(seriesIndex, RangeF(xFirst, xLast), scalingIndex, points);
}


bool AbstractFileReader::readSeries(int seriesIndex, RangeF xValue, int scalingIndex, QVector<QPointF> &points)
{
    if (!isValidSeriesIndex(seriesIndex) || !isValidXValue(xValue)) {
        return true;
    }

    Range xIndex = xIndexByValue(xValue, scalingIndex);
    xIndex.intersect(Range(0, m_cacheInfo.xMaxIndex(scalingIndex)));

    return readSeries(seriesIndex, xIndex, scalingIndex, points);
}

bool AbstractFileReader::readSeries(int seriesIndex, qint64 xFirst, qint64 xLast,
                                    int scalingIndex, QVector<QPointF> &points)
{
    return readSeries(seriesIndex, Range(xFirst, xLast), scalingIndex, points);
}

bool AbstractFileReader::readSeries(int seriesIndex, Range xIndex, int scalingIndex, QVector<QPointF> &points)
{
    if (!isValidSeriesIndex(seriesIndex) || !isValidXIndex(xIndex, scalingIndex)) {
        return true;
    }

    if (scalingIndex < 1) {
        return readFile(seriesIndex, xIndex, points);
    }

    qreal xStep = m_scaleInfo.xStep(scalingIndex);

    QVector<qreal> yValues;
    if (!readCache(seriesIndex, xIndex, scalingIndex, yValues)) {
        points.clear();
        return false;
    }

    points.resize(yValues.size());
    qreal xValue = m_xLimit.min_() + (xIndex.min_() + 0.5) * xStep;

    for (int i = 0; i < yValues.size(); i += 2) {
        points[i].setX(xValue);
        points[i].setY(yValues.at(i));

        points[i + 1].setX(xValue);
        points[i + 1].setY(yValues.at(i + 1));

        xValue += xStep;
    }

    return true;
}

bool AbstractFileReader::readSeriesY(int seriesIndex, const Range &xIndex, QVector<qreal> &y)
{
    if (!isValidSeriesIndex(seriesIndex) || !isValidXIndex(xIndex, 0)) {
        return true;
    }

    return readFile(seriesIndex, xIndex, y);
}

Range AbstractFileReader::xIndexByValue(RangeF xValue, int scalingIndex) const
{
    xValue -= m_xLimit.min_();
    xValue /= m_xStep;

    Range result(std::llround(xValue.min_()), std::llround(xValue.max_()));

    if (scalingIndex > 0) {
        result /= m_scaleInfo.pointsCombinedCount(scalingIndex);
    }

    return result;
}

QVector<bool> AbstractFileReader::cacheFilesExist() const
{
    QVector<bool> result;

    for (int seriesIndex = 0; seriesIndex < m_seriesCount; ++seriesIndex) {
        result.append(cacheExists(seriesIndex));
    }

    return result;
}

qint64 AbstractFileReader::pointsCount() const
{
    return m_pointsCount;
}

const ScaleInfo *AbstractFileReader::scaleInfo() const
{
    return &m_scaleInfo;
}

int AbstractFileReader::seriesCount() const
{
    return m_seriesCount;
}

const QString &AbstractFileReader::seriesName(int index) const
{
    return m_seriesNameList.at(index);
}

const QStringList &AbstractFileReader::seriesNameList() const
{
    return m_seriesNameList;
}

const RangeF &AbstractFileReader::xLimit() const
{
    return m_xLimit;
}

RangeF AbstractFileReader::yLimit(int seriesIndex)
{
    if (!isValidSeriesIndex(seriesIndex)) {
        return RangeF();
    }

    QVector<qreal> y;
    readCache(seriesIndex, Range(), m_cacheInfo.maxValidScalingIndex(), y);

    return RangeF(y.at(0), y.at(1));
}

qreal AbstractFileReader::xMax() const
{
    return m_xLimit.max_();
}

qreal AbstractFileReader::xMin() const
{
    return m_xLimit.min_();
}

qreal AbstractFileReader::xStep() const
{
    return m_xStep;
}

bool AbstractFileReader::cacheExists() const
{
    for (int seriesIndex = 0; seriesIndex < m_seriesCount; ++seriesIndex) {
        if (m_selectedSeries.at(seriesIndex) && !cacheExists(seriesIndex)) {
            return false;
        }
    }

    return true;
}

bool AbstractFileReader::cacheExists(int seriesIndex) const
{
    QFileInfo info(m_cacheInfo.fileName(seriesIndex));
    return ((info.size() == m_cacheInfo.fileSize()) && (info.lastModified() > QFileInfo(*m_file).lastModified()));
}

bool AbstractFileReader::createCache(bool forced)
{
    bool success = true;

    QVector<QFuture<bool>> futures;
    QVector<QFilePointer> cacheFiles;

    qint64 totalBytes = 0;
    qint64 currentBytes = 0;

    for (int seriesIndex = 0; seriesIndex < m_seriesCount; ++seriesIndex) {
        if ((m_selectedSeries.at(seriesIndex) == true) && (forced || !cacheExists(seriesIndex))) {
            cacheFiles.append(QFilePointer(new QFile(m_cacheInfo.fileName(seriesIndex))));
            cacheFiles.last()->open(QIODevice::WriteOnly);
            cacheFiles.last()->resize(m_cacheInfo.fileSize());

            totalBytes += m_cacheInfo.fileSize();
            futures.append(QtConcurrent::run(this, &AbstractFileReader::createCacheForOneSeries,
                                             seriesIndex, cacheFiles.last(), forced));
        }
    }

    while (currentBytes != totalBytes) {
        QThread::msleep(10);
        currentBytes = 0;
        for (int i = 0; i < cacheFiles.size(); ++i) {
            currentBytes += cacheFiles.at(i)->pos();
        }
        emit cacheCreationProgress(100.0 * currentBytes / totalBytes);
    }

    for (int i = 0; i < futures.size(); ++ i) {
        if (futures[i].result() == false) {
            success = false;
        }
    }

    return success;
}

bool AbstractFileReader::createCacheForOneSeries(int seriesIndex, QFilePointer outFile, bool forced)
{
//    if ((seriesIndex < 0) || (seriesIndex >= m_seriesCount)) {
//        return false;
//    }

//    if ((!forced) && (cacheExists(seriesIndex))) {
//        return true;
//    }
    Q_UNUSED(forced)

    bool success = true;

    if (!createCacheForOneSeriesFirstIteration(seriesIndex, outFile)) {
        success = false;
    }

    if (!createCacheForOneSeriesNextIterations(seriesIndex, outFile)) {
        success = false;
    }

    return success;
}

bool AbstractFileReader::createCacheForOneSeriesNextIterations(int seriesIndex, QFilePointer outFile)
{
    QFilePointer inFile(new QFile(m_cacheInfo.fileName(seriesIndex)));
    inFile->open(QIODevice::ReadOnly);

    QDataStream in(inFile.get());
    QDataStream out(outFile.get());

    int blockSize = m_scaleInfo.base();
    qint64 pointsCount = ceilDivision(m_pointsCount, m_scaleInfo.firstIterationBase());
    QVector<qreal> yMin(blockSize), yMax(blockSize);

    while (pointsCount > 1) {
        qint64 blockCount = pointsCount / blockSize;
        int lastBlockSize = pointsCount % blockSize;

        for (qint64 blockIndex = 0; blockIndex < blockCount; ++blockIndex) {
            for (int i = 0; i < blockSize; ++i) {
                in >> yMin[i] >> yMax[i];
            }
            out << *std::min_element(yMin.begin(), yMin.end()) << *std::max_element(yMax.begin(), yMax.end());
        }

        if (lastBlockSize != 0) {
            for (int i = 0; i < lastBlockSize; ++i) {
                in >> yMin[i] >> yMax[i];
            }
            out << *std::min_element(yMin.begin(), yMin.begin() + lastBlockSize)
                << *std::max_element(yMax.begin(), yMax.begin() + lastBlockSize);

            ++blockCount;
        }
        pointsCount = blockCount;

        // write buffer to disk
        outFile->flush();
        // reset buffer
        inFile->seek(inFile->pos() - 1);
        inFile->skip(1);
    }

    return ((in.status() == QDataStream::Ok) && (out.status() == QDataStream::Ok));
}

bool AbstractFileReader::deleteCache() const
{
    for (int seriesIndex = 0; seriesIndex < m_seriesCount; ++seriesIndex) {
        if (!deleteCache(seriesIndex)) {
            return false;
        }
    }

    return true;
}

bool AbstractFileReader::deleteCache(int seriesIndex) const
{
    QString fileName = m_cacheInfo.fileName(seriesIndex);
    return !QFileInfo::exists(fileName) || QFile::remove(fileName);
}

bool AbstractFileReader::readCache(int seriesIndex, Range xIndex, int scalingIndex, QVector<qreal> &values)
{
    QFile inFile(m_cacheInfo.fileName(seriesIndex));
    inFile.open(QIODevice::ReadOnly);
    inFile.seek(m_cacheInfo.byteOffset(scalingIndex, xIndex.min_()));

    QDataStream inStream(&inFile);
    inStream.setVersion(QDataStream::Qt_5_12);

    values.resize((xIndex.length() + 1) * m_cacheInfo.cachePointSize());
    for (qint64 i = 0; i < values.size(); ++i) {
        inStream >> values[i];
    }

    if (inStream.status() != QDataStream::Ok) {
        values.clear();
        return false;
    }

    return true;
}

void AbstractFileReader::initInfo()
{
    m_scaleInfo.init(m_xStep);
    m_cacheInfo.init(m_file->fileName(), m_pointsCount, m_scaleInfo);
}

void AbstractFileReader::setFilePositionToPoint(qint64 pointIndex)
{
    m_file->seek(m_sizeOfMetadata + pointIndex * m_sizeOfPoint);
}

bool AbstractFileReader::isValidSeriesIndex(int seriesIndex) const
{
    return (0 <= seriesIndex) && (seriesIndex < m_seriesCount);
}

bool AbstractFileReader::isValidXIndex(const Range &xIndex, int scalingIndex) const
{
    return xIndex.isValid() && Range(0, m_cacheInfo.xMaxIndex(scalingIndex)).contains(xIndex);
}

bool AbstractFileReader::isValidXValue(RangeF xValue) const
{
    return xValue.isValid() && m_xLimit.isIntersects(xValue);
}

qreal AbstractFileReader::xValueByIndex(qint64 index) const
{
    return m_xLimit.min_() + m_xStep * index;
}

qint64 AbstractFileReader::xIndexByValue(qreal value) const
{
    return std::llround((value - m_xLimit.min_()) / m_xStep);
}

FileReader::FileReader(QObject *parent)
    : AbstractFileReader(parent)
{

}

FileReader::~FileReader()
{

}

bool FileReader::openFile(const QString &fileName)
{
    AbstractFileReader::openFile(fileName);

    m_sizeOfPoint = m_seriesCount * sizeof(qreal);
    m_pointsCount = (m_file->size() - m_sizeOfMetadata) / m_sizeOfPoint;
    m_xMaxIndex = m_pointsCount - 1;
    m_xLimit.setMax(m_xLimit.min_() + m_xMaxIndex * m_xStep);

    initInfo();
    return !hasError();
}

bool FileReader::createCacheForOneSeriesFirstIteration(int seriesIndex, QFilePointer outFile)
{
    QFilePointer inFile(new QFile(m_file->fileName()));
    inFile->open(QIODevice::ReadOnly);

    QStreamPointer inStream(new QDataStream(inFile.get()));
    QStreamPointer outStream(new QDataStream(outFile.get()));

    // converting one block of points to two points with the same x value
    int blockSize = m_scaleInfo.firstIterationBase();
    int lastBlockSize = m_pointsCount % blockSize;
    qint64 blockCount = m_pointsCount / blockSize;

    inFile->seek(m_sizeOfMetadata);

    ReadBuffer buffer;
    buffer.yValues.resize(blockSize);

    for (qint64 i = 0; i < blockCount; ++i) {
        readMinMaxSeriesAndWriteToStream(seriesIndex, buffer, inStream, outStream);
    }

    if (lastBlockSize != 0) {
        buffer.yValues.resize(lastBlockSize);
        readMinMaxSeriesAndWriteToStream(seriesIndex, buffer, inStream, outStream);
    }

    outFile->flush();
    return (inStream->status() == QDataStream::Ok) && (outStream->status() == QDataStream::Ok);
}


bool FileReader::readFile(int seriesIndex, Range xIndex, QVector<QPointF> &points)
{
    qint64 xCount = xIndex.length() + 1;
    points.resize(xCount);

    setFilePositionToPoint(xIndex.min_());

    qreal x = m_xLimit.min_() + m_xStep * (xIndex.min_() - 1);
    qreal y;

    for (qint64 i = 0; i < xCount; ++i) {
        x += m_xStep;
        readY(seriesIndex, y);

        points[i].setX(x);
        points[i].setY(y);
    }

    if (hasError()) {
        points.clear();
        return false;
    }

    return true;
}

bool FileReader::readFile(int seriesIndex, Range xIndex, QVector<qreal> &y)
{
    qint64 xCount = xIndex.length() + 1;
    y.resize(xCount);

    setFilePositionToPoint(xIndex.min_());

    for (qint64 i = 0; i < xCount; ++i) {
        readY(seriesIndex, y[i]);
    }

    if (hasError()) {
        y.clear();
        return false;
    }

    return true;
}

void FileReader::readMinMaxSeriesAndWriteToStream(int seriesIndex, ReadBuffer &buffer,
        QStreamPointer &inStream, QStreamPointer &outStream)
{
    qint64 pointsCount = buffer.yValues.size();

    for (qint64 pointIndex = 0; pointIndex < pointsCount; ++pointIndex) {
        readY(seriesIndex, buffer.yValues[pointIndex], inStream);
    }

    const auto minmax = std::minmax_element(buffer.yValues.begin(), buffer.yValues.end());
    *outStream << *minmax.first << *minmax.second;
}

void FileReader::readY(int seriesIndex, qreal &y)
{
    readY(seriesIndex, y, m_dataStream);
}

void FileReader::readY(int seriesIndex, qreal &y, QStreamPointer &inStream)
{
    inStream->skipRawData(seriesIndex * sizeof(qreal));
    *inStream >> y;
    inStream->skipRawData((m_seriesCount - seriesIndex - 1) * sizeof(qreal));
}

ZipFileReader::ZipFileReader(QObject *parent)
    : AbstractFileReader(parent)
{

}

ZipFileReader::~ZipFileReader()
{

}

bool ZipFileReader::openFile(const QString &fileName)
{
    AbstractFileReader::openFile(fileName);

    m_sizeOfPoint = (m_seriesCount + 1) * sizeof(qreal);
    m_zipPointsCount = (m_file->size() - m_sizeOfMetadata) / m_sizeOfPoint;

    readXIndexByZipIndex(m_zipPointsCount - 1, m_xMaxIndex);

    m_pointsCount = m_xMaxIndex + 1;
    m_xLimit.setMax(m_xLimit.min_() + m_xMaxIndex * m_xStep);

    initInfo();
    return !hasError();
}

bool ZipFileReader::createCacheForOneSeriesFirstIteration(int seriesIndex, QFilePointer outFile)
{
    QFilePointer inFile(new QFile(m_file->fileName()));
    inFile->open(QIODevice::ReadOnly);

    QStreamPointer inStream(new QDataStream(inFile.get()));
    QStreamPointer outStream(new QDataStream(outFile.get()));

    // converting one block of points to two points with the same x value
    int blockSize = m_scaleInfo.firstIterationBase();
    int lastBlockSize = m_pointsCount % blockSize;
    qint64 blockCount = m_pointsCount / blockSize;

    inFile->seek(m_sizeOfMetadata);

    ZipReadBuffer buffer;
    buffer.xIndex = 0;
    buffer.yValues.resize(blockSize);
    readXY(seriesIndex, buffer.xIndexRight, buffer.yValueRight, inStream);
    buffer.yValueLeft = buffer.yValueRight;

    for (qint64 i = 0; i < blockCount; ++i) {
        readMinMaxSeriesAndWriteToStream(seriesIndex, buffer, inStream, outStream);
    }

    if (lastBlockSize != 0) {
        buffer.yValues.resize(lastBlockSize);
        readMinMaxSeriesAndWriteToStream(seriesIndex, buffer, inStream, outStream);
    }

    outFile->flush();
    return (inStream->status() == QDataStream::Ok) && (outStream->status() == QDataStream::Ok);
}

bool ZipFileReader::readFile(int seriesIndex, Range xIndex, QVector<QPointF> &points)
{
    qint64 xCount = xIndex.length() + 1;
    points.resize(xCount);

    setFilePositionToPoint(xZipIndexByIndex(xIndex.min_()));

    qint64 xIndexCurr = xIndex.min_() - 1;
    qint64 xIndexRight;
    qreal xValue = m_xLimit.min_() + m_xStep * (xIndex.min_() - 1);
    qreal yValueLeft, yValueRight;

    readXY(seriesIndex, xIndexRight, yValueRight);
    yValueLeft = yValueRight;

    for (qint64 i = 0; i < xCount; ++i) {
        ++xIndexCurr;
        xValue += m_xStep;

        if (xIndexRight < xIndexCurr) {
            yValueLeft = yValueRight;
            readXY(seriesIndex, xIndexRight, yValueRight);
        }

        if (xIndexRight == xIndexCurr) {
            points[i].setX(xValue);
            points[i].setY(yValueRight);
        } else {
            points[i].setX(xValue);
            points[i].setY(yValueLeft);
        }
    }

    if (hasError()) {
        points.clear();
        return false;
    }

    return true;
}

bool ZipFileReader::readFile(int seriesIndex, Range xIndex, QVector<qreal> &y)
{
    qint64 xCount = xIndex.length() + 1;
    y.resize(xCount);

    setFilePositionToPoint(xZipIndexByIndex(xIndex.min_()));

    qint64 xIndexCurr = xIndex.min_() - 1;
    qint64 xIndexRight;
    qreal yValueLeft, yValueRight;

    readXY(seriesIndex, xIndexRight, yValueRight);
    yValueLeft = yValueRight;

    for (qint64 i = 0; i < xCount; ++i) {
        ++xIndexCurr;

        if (xIndexRight < xIndexCurr) {
            yValueLeft = yValueRight;
            readXY(seriesIndex, xIndexRight, yValueRight);
        }

        if (xIndexRight == xIndexCurr) {
            y[i] = yValueRight;
        } else {
            y[i] = yValueLeft;
        }
    }

    if (hasError()) {
        y.clear();
        return false;
    }

    return true;
}

void ZipFileReader::readMinMaxSeriesAndWriteToStream(int seriesIndex, ZipReadBuffer &buffer,
        QStreamPointer &inStream, QStreamPointer &outStream)
{
    qint64 pointsCount = buffer.yValues.size();

    for (qint64 pointIndex = 0; pointIndex < pointsCount; ++pointIndex) {
        ++buffer.xIndex;

        if (buffer.xIndexRight < buffer.xIndex) {
            buffer.yValueLeft = buffer.yValueRight;
            readXY(seriesIndex, buffer.xIndexRight, buffer.yValueRight, inStream);
        }

        if (buffer.xIndexRight == buffer.xIndex) {
            buffer.yValues[pointIndex] = buffer.yValueRight;
        } else {
            buffer.yValues[pointIndex] = buffer.yValueLeft;
        }
    }

    const auto minmax = std::minmax_element(buffer.yValues.begin(), buffer.yValues.end());
    *outStream << *minmax.first << *minmax.second;
}

void ZipFileReader::readXY(int seriesIndex, qint64 &x, qreal &y)
{
    readXY(seriesIndex, x, y, m_dataStream);
}

void ZipFileReader::readXY(int seriesIndex, qint64 &x, qreal &y, QStreamPointer &inStream)
{
    *inStream >> x;
    inStream->skipRawData(seriesIndex * sizeof(qreal));
    *inStream >> y;
    inStream->skipRawData((m_seriesCount - seriesIndex - 1) * sizeof(qreal));
}

void ZipFileReader::readXIndexByZipIndex(qint64 xZipIndex, qint64 &xIndex)
{
    setFilePositionToPoint(xZipIndex);
    *m_dataStream >> xIndex;
}

// todo: add cache
qint64 ZipFileReader::xZipIndexByIndex(qint64 xIndex)
{
    qint64 zipLeft = 0;
    qint64 zipRight = m_zipPointsCount - 1;

    // include this check to cache
    if (xIndex == 0) {
        return zipLeft;
    } else if (xIndex == m_xMaxIndex) {
        return zipRight;
    }

    qint64 zipMiddle;
    qint64 xIndexValue;

    do {
        zipMiddle = zipLeft + (zipRight - zipLeft) / 2; // no overflow
        readXIndexByZipIndex(zipMiddle, xIndexValue);

        if (xIndex == xIndexValue) {
            return zipMiddle;
        }

        if (xIndex < xIndexValue) {
            zipRight = zipMiddle;
        } else {
            zipLeft = zipMiddle;
        }
    } while (zipLeft + 1 != zipRight);

    return zipLeft;
}

OldFileReader::OldFileReader(QObject *parent)
    : FileReader(parent)
{

}

OldFileReader::~OldFileReader()
{

}

bool OldFileReader::openFile(const QString &fileName)
{
    AbstractFileReader::openFile(fileName); // clazy:exclude=skipped-base-method
    m_dataStream->setByteOrder(QDataStream::LittleEndian); // diff

    m_sizeOfPoint = (m_seriesCount + 1) * sizeof(qreal); // diff
    m_pointsCount = (m_file->size() - m_sizeOfMetadata) / m_sizeOfPoint;
    m_xMaxIndex = m_pointsCount - 1;
    m_xLimit.setMax(m_xLimit.min_() + m_xMaxIndex * m_xStep);

    initInfo();
    return !hasError();
}

bool OldFileReader::createCacheForOneSeriesFirstIteration(int seriesIndex, QFilePointer outFile)
{
    QFilePointer inFile(new QFile(m_file->fileName()));
    inFile->open(QIODevice::ReadOnly);

    QStreamPointer inStream(new QDataStream(inFile.get()));
    QStreamPointer outStream(new QDataStream(outFile.get()));

    inFile->seek(m_sizeOfMetadata);

    // converting one block of points to two points with the same x value
    int blockSize = m_scaleInfo.firstIterationBase();
    int lastBlockSize = m_pointsCount % blockSize;
    qint64 blockCount = m_pointsCount / blockSize;

    ReadBuffer buffer;
    buffer.yValues.resize(blockSize);

    inStream->setByteOrder(QDataStream::LittleEndian); // diff

    for (qint64 i = 0; i < blockCount; ++i) {
        readMinMaxSeriesAndWriteToStream(seriesIndex, buffer, inStream, outStream);
    }

    if (lastBlockSize != 0) {
        buffer.yValues.resize(lastBlockSize);
        readMinMaxSeriesAndWriteToStream(seriesIndex, buffer, inStream, outStream);
    }

    outFile->flush();
    return (inStream->status() == QDataStream::Ok) && (outStream->status() == QDataStream::Ok);
}


bool OldFileReader::readFile(int seriesIndex, Range xIndex, QVector<QPointF> &points)
{
    qint64 xCount = xIndex.length() + 1;
    points.resize(xCount);

    qreal x = m_xLimit.min_() + m_xStep * (xIndex.min_() - 1);
    qreal y;

    setFilePositionToPoint(xIndex.min_());

    for (qint64 i = 0; i < xCount; ++i) {
        x += m_xStep;
        readY(seriesIndex, y);

        points[i].setX(x);
        points[i].setY(y);
    }

    if (hasError()) {
        points.clear();
        return false;
    }

    return true;
}

bool OldFileReader::readFile(int seriesIndex, Range xIndex, QVector<qreal> &y)
{
    qint64 xCount = xIndex.length() + 1;
    y.resize(xCount);

    setFilePositionToPoint(xIndex.min_());

    for (qint64 i = 0; i < xCount; ++i) {
        readY(seriesIndex, y[i]);
    }

    if (hasError()) {
        y.clear();
        return false;
    }

    return true;
}

void OldFileReader::readMinMaxSeriesAndWriteToStream(int seriesIndex, ReadBuffer &buffer,
        QStreamPointer &inStream, QStreamPointer &outStream)
{
    qint64 pointsCount = buffer.yValues.size();

    for (qint64 pointIndex = 0; pointIndex < pointsCount; ++pointIndex) {
        readY(seriesIndex, buffer.yValues[pointIndex], inStream);
    }

    const auto minmax = std::minmax_element(buffer.yValues.begin(), buffer.yValues.end());
    *outStream << *minmax.first << *minmax.second;
}

void OldFileReader::readY(int seriesIndex, qreal &y)
{
    readY(seriesIndex, y, m_dataStream);
}

void OldFileReader::readY(int seriesIndex, qreal &y, QStreamPointer &inStream)
{
//    inStream->skipRawData((seriesIndex + 1) * sizeof(qreal)); // diff
//    *inStream >> y;
//    inStream->skipRawData((m_seriesCount - seriesIndex - 1) * sizeof(qreal));
    inStream->skipRawData(sizeof(qreal));
    FileReader::readY(seriesIndex, y, inStream);
}

OldZipFileReader::OldZipFileReader(QObject *parent)
    : ZipFileReader(parent)
{

}

OldZipFileReader::~OldZipFileReader()
{

}

bool OldZipFileReader::openFile(const QString &fileName)
{
    AbstractFileReader::openFile(fileName); // clazy:exclude=skipped-base-method
    m_dataStream->setByteOrder(QDataStream::LittleEndian); // diff

    m_sizeOfPoint = sizeof(qint64) + (m_seriesCount) * sizeof(qreal); // diff
    m_zipPointsCount = (m_file->size() - m_sizeOfMetadata) / m_sizeOfPoint;

    readXIndexByZipIndex(m_zipPointsCount - 1, m_xMaxIndex);

    m_pointsCount = m_xMaxIndex + 1;
    m_xLimit.setMax(m_xLimit.min_() + m_xMaxIndex * m_xStep);

    initInfo();
    return !hasError();
}

bool OldZipFileReader::createCacheForOneSeriesFirstIteration(int seriesIndex, QFilePointer outFile)
{
    QFilePointer inFile(new QFile(m_file->fileName()));
    inFile->open(QIODevice::ReadOnly);

    QStreamPointer inStream(new QDataStream(inFile.get()));
    inStream->setByteOrder(QDataStream::LittleEndian); // diff
    QStreamPointer outStream(new QDataStream(outFile.get()));

    // converting one block of points to two points with the same x value
    int blockSize = m_scaleInfo.firstIterationBase();
    int lastBlockSize = m_pointsCount % blockSize;
    qint64 blockCount = m_pointsCount / blockSize;

    inFile->seek(m_sizeOfMetadata);

    ZipReadBuffer buffer;
    buffer.xIndex = 0;
    buffer.yValues.resize(blockSize);
    readXY(seriesIndex, buffer.xIndexRight, buffer.yValueRight, inStream);
    buffer.yValueLeft = buffer.yValueRight;

    for (qint64 i = 0; i < blockCount; ++i) {
        readMinMaxSeriesAndWriteToStream(seriesIndex, buffer, inStream, outStream);
    }

    if (lastBlockSize != 0) {
        buffer.yValues.resize(lastBlockSize);
        readMinMaxSeriesAndWriteToStream(seriesIndex, buffer, inStream, outStream);
    }

    outFile->flush();
    return (inStream->status() == QDataStream::Ok) && (outStream->status() == QDataStream::Ok);
}

bool OldZipFileReader::readFile(int seriesIndex, Range xIndex, QVector<QPointF> &points)
{
    qint64 xCount = xIndex.length() + 1;
    points.resize(xCount);

    setFilePositionToPoint(xZipIndexByIndex(xIndex.min_()));

    qint64 xIndexCurr = xIndex.min_() - 1;
    qint64 xIndexRight;
    qreal xValue = m_xLimit.min_() + m_xStep * (xIndex.min_() - 1);
    qreal yValueLeft, yValueRight;

    readXY(seriesIndex, xIndexRight, yValueRight);
    yValueLeft = yValueRight;

    for (qint64 i = 0; i < xCount; ++i) {
        ++xIndexCurr;
        xValue += m_xStep;

        if (xIndexRight < xIndexCurr) {
            yValueLeft = yValueRight;
            readXY(seriesIndex, xIndexRight, yValueRight);
        }

        if (xIndexRight == xIndexCurr) {
            points[i].setX(xValue);
            points[i].setY(yValueRight);
        } else {
            points[i].setX(xValue);
            points[i].setY(yValueLeft);
        }
    }

    if (hasError()) {
        points.clear();
        return false;
    }

    return true;
}

bool OldZipFileReader::readFile(int seriesIndex, Range xIndex, QVector<qreal> &y)
{
    qint64 xCount = xIndex.length() + 1;
    y.resize(xCount);

    setFilePositionToPoint(xZipIndexByIndex(xIndex.min_()));

    qint64 xIndexCurr = xIndex.min_() - 1;
    qint64 xIndexRight;
    qreal yValueLeft, yValueRight;

    readXY(seriesIndex, xIndexRight, yValueRight);
    yValueLeft = yValueRight;

    for (qint64 i = 0; i < xCount; ++i) {
        ++xIndexCurr;

        if (xIndexRight < xIndexCurr) {
            yValueLeft = yValueRight;
            readXY(seriesIndex, xIndexRight, yValueRight);
        }

        if (xIndexRight == xIndexCurr) {
            y[i] = yValueRight;
        } else {
            y[i] = yValueLeft;
        }
    }

    if (hasError()) {
        y.clear();
        return false;
    }

    return true;
}

void OldZipFileReader::readMinMaxSeriesAndWriteToStream(int seriesIndex, ZipReadBuffer &buffer,
        QStreamPointer &inStream, QStreamPointer &outStream)
{
    qint64 pointsCount = buffer.yValues.size();

    for (qint64 pointIndex = 0; pointIndex < pointsCount; ++pointIndex) {
        ++buffer.xIndex;

        if (buffer.xIndexRight < buffer.xIndex) {
            buffer.yValueLeft = buffer.yValueRight;
            readXY(seriesIndex, buffer.xIndexRight, buffer.yValueRight, inStream);
        }

        if (buffer.xIndexRight == buffer.xIndex) {
            buffer.yValues[pointIndex] = buffer.yValueRight;
        } else {
            buffer.yValues[pointIndex] = buffer.yValueLeft;
        }
    }

    const auto minmax = std::minmax_element(buffer.yValues.begin(), buffer.yValues.end());
    *outStream << *minmax.first << *minmax.second;
}

void OldZipFileReader::readXY(int seriesIndex, qint64 &x, qreal &y)
{
    readXY(seriesIndex, x, y, m_dataStream);
}

void OldZipFileReader::readXY(int seriesIndex, qint64 &x, qreal &y, QStreamPointer &inStream)
{
    // diff
    qreal xValue;
    *inStream >> xValue;
    x = xIndexByValue(xValue);

    inStream->skipRawData(seriesIndex * sizeof(qreal));
    *inStream >> y;
    inStream->skipRawData((m_seriesCount - seriesIndex - 1) * sizeof(qreal));
}

void OldZipFileReader::readXIndexByZipIndex(qint64 xZipIndex, qint64 &xIndex)
{
    setFilePositionToPoint(xZipIndex);
    // diff
    qreal xValue;
    *m_dataStream >> xValue;
    xIndex = xIndexByValue(xValue);
}

// todo: add cache
qint64 OldZipFileReader::xZipIndexByIndex(qint64 xIndex)
{
    qint64 zipLeft = 0;
    qint64 zipRight = m_zipPointsCount - 1;

    // include this check to cache
    if (xIndex == 0) {
        return zipLeft;
    } else if (xIndex == m_xMaxIndex) {
        return zipRight;
    }

    qint64 zipMiddle;
    qint64 xIndexValue;

    do {
        zipMiddle = zipLeft + (zipRight - zipLeft) / 2; // no overflow
        readXIndexByZipIndex(zipMiddle, xIndexValue);

        if (xIndex == xIndexValue) {
            return zipMiddle;
        }

        if (xIndex < xIndexValue) {
            zipRight = zipMiddle;
        } else {
            zipLeft = zipMiddle;
        }
    } while (zipLeft + 1 != zipRight);

    return zipLeft;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////// AbstractFileWriter ////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

const QString AbstractFileWriter::m_fileExtension = "sgnl2d";

AbstractFileWriter::AbstractFileWriter(QStringList seriesNames, qreal xMin, qreal xStep)
{
    m_metadata.seriesNameList = seriesNames;
    m_metadata.xMin = xMin;
    m_metadata.xStep = xStep;

    m_seriesCount = seriesNames.size();

    if (seriesNames.size() == 0) {
        m_seriesCount = 1;
        m_metadata.seriesNameList.append(AbstractFileIO::defaultSeriesName());
    }

    if (xStep <= 0) {
        m_metadata.xStep = 1;
        // todo: log error
        // invalidArgument xStep must be positive, val = xStep
    }
}

AbstractFileWriter::~AbstractFileWriter()
{

}

void AbstractFileWriter::closeFile()
{
    m_file->close();
    m_file->setFileName("");
}

const QString &AbstractFileWriter::fileExtension()
{
    return m_fileExtension;
}

bool AbstractFileWriter::openFile(const QString &fileName)
{
    m_file->setFileName(QString("%1.%2").arg(fileName, m_fileExtension));
    m_file->open(QIODevice::WriteOnly);

    *m_dataStream << m_metadata;

    return !hasError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// FileWriter ////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

FileWriter::FileWriter(QStringList seriesNames, qreal xMin, qreal xStep)
    : AbstractFileWriter(seriesNames, xMin, xStep)
{
    m_metadata.fileFormat = FileFormat::General;
}

FileWriter::~FileWriter()
{

}

void FileWriter::writePoint(const QVector<qreal> &point)
{
    for (int i = 0; i < m_seriesCount; ++i) {
        *m_dataStream << point.at(i);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// ZipFileWriter //////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

ZipFileWriter::ZipFileWriter(QStringList seriesNames, qreal xMin, qreal xStep)
    : AbstractFileWriter(seriesNames, xMin, xStep)
{
    m_metadata.fileFormat = FileFormat::RLEzip;
    m_currentXIndex = -1;
    m_lastYChanged = true;
}

ZipFileWriter::~ZipFileWriter()
{
    if (!m_lastYChanged) {
        *m_dataStream << m_currentXIndex;

        for (int i = 0; i < m_seriesCount; ++i) {
            *m_dataStream << m_lastYValue.at(i);
        }
    }
}

void ZipFileWriter::writePoint(const QVector<qreal> &point)
{
    ++m_currentXIndex;

    if (m_lastYValue != point) {
        *m_dataStream << m_currentXIndex;

        for (int i = 0; i < m_seriesCount; ++i) {
            *m_dataStream << point.at(i);
        }

        m_lastYValue = point;
        m_lastYChanged = true;
    } else {
        m_lastYChanged = false;
    }
}

}
