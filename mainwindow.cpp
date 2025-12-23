#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QSettings>
#include <QDir>

#include <QStringListModel>

#include <QUrl>

#include <QFile>
#include <QTextStream>
#include <QRegularExpression>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
       loadSettings();
    mp3Model = new QStringListModel(this);
    ui->listView->setModel(mp3Model);

    // 起動時に lineEdit のパスから読み込み
    updateMp3List(ui->lineEdit->text());
    restoreLastPlayed();     // ← そのあとで！

    ui->textEdit->setReadOnly(true);
   // ui->textEdit->setStyleSheet("background:white; color:black;");

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    audioOutput = new QAudioOutput(this);
    player = new QMediaPlayer(this);
    player->setAudioOutput(audioOutput);



    audioOutput->setVolume(0.8);
#else
    player = new QMediaPlayer(this);
    player->setVolume(80);
#endif

    connect(player, &QMediaPlayer::positionChanged,
            this, &MainWindow::updateSubtitle);


    ui->textEdit->setReadOnly(true);
    ui->textEdit->setAlignment(Qt::AlignCenter);
    ui->textEdit->setStyleSheet(
        "font-size: 24px; background: black; color: white;"
        );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("フォルダを選択"),
        ui->lineEdit->text().isEmpty()
            ? QDir::homePath()
            : ui->lineEdit->text()
        );

    if (dir.isEmpty())
        return;

    ui->lineEdit->setText(dir);
    saveSettings();

    connect(player, &QMediaPlayer::positionChanged,
            this, &MainWindow::updateSubtitle);
}

void MainWindow::saveSettings()
{
    QSettings settings("MyCompany", "MyApp"); // ←名前は自由
    settings.setValue("path/targetDir", ui->lineEdit->text());
}

void MainWindow::loadSettings()
{
    QSettings settings("MyCompany", "MyApp");
    QString dir = settings.value("path/targetDir", "").toString();

    if (!dir.isEmpty())
        ui->lineEdit->setText(dir);
}

void MainWindow::updateMp3List(const QString &dirPath)
{
    if (dirPath.isEmpty())
        return;

    QDir dir(dirPath);
    if (!dir.exists())
        return;

    QStringList filters;
    filters << "*.mp3" << "*.MP3";

    QStringList files = dir.entryList(
        filters,
        QDir::Files | QDir::Readable,
        QDir::Name   // ソート
        );

    mp3Model->setStringList(files);
}


void MainWindow::on_listView_doubleClicked(const QModelIndex &index)
{
    ui->listView->setCurrentIndex(index);   // ← 強調表示

    QString fileName = mp3Model->data(index, Qt::DisplayRole).toString();
    QString fullPath = QDir(ui->lineEdit->text()).filePath(fileName);

   // qDebug() << "MP3 double clicked:" << fullPath;
    player->setSource(QUrl::fromLocalFile(fullPath));
    player->play();

      loadSrt(fullPath);   // ← 追加
    saveLastPlayed(fileName);   // ← 保存


}

void MainWindow::saveLastPlayed(const QString &fileName)
{
    QSettings settings("MyCompany", "QtMp3Srt");
    settings.setValue("player/lastFile", fileName);
}

void MainWindow::restoreLastPlayed()
{
    QSettings settings("MyCompany", "QtMp3Srt");
    QString lastFile = settings.value("player/lastFile").toString();

    if (lastFile.isEmpty())
        return;

    QStringList files = mp3Model->stringList();
    int row = files.indexOf(lastFile);

    if (row >= 0) {
        QModelIndex index = mp3Model->index(row);
        ui->listView->setCurrentIndex(index);
        ui->listView->scrollTo(index);
    }
}

static qint64 timeToMs(const QString &time)
{
    // "00:00:06,840"
    QString t = time.trimmed();
    QRegularExpression re("(\\d+):(\\d+):(\\d+),(\\d+)");
    auto m = re.match(t);
    if (!m.hasMatch()) {
     //   qDebug() << "time parse error:" << time;
        return 0;
    }

    int h  = m.captured(1).toInt();
    int m_ = m.captured(2).toInt();
    int s  = m.captured(3).toInt();
    int ms = m.captured(4).toInt();

    return ((h * 3600 + m_ * 60 + s) * 1000) + ms;
}


void MainWindow::loadSrt(const QString &mp3Path)
{
    subtitles.clear();

    QString srtPath = mp3Path;
    srtPath.replace(QRegularExpression("\\.mp3$", QRegularExpression::CaseInsensitiveOption),
                    ".srt");
//qDebug() << "loadSrt called with:" << mp3Path;
    QFile file(srtPath);
   // if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
   //     return;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
     //   qDebug() << "FAILED to open SRT";
        return;
    }
   // qDebug() << "SRT opened OK"<<srtPath;

    QTextStream in(&file);
    //in.setCodec("UTF-8");

    while (!in.atEnd()) {
        QString line = in.readLine(); // index
        if (line.trimmed().isEmpty())
            continue;

        QString timeLine = in.readLine(); // time
        QString text;

       // qDebug() << "Time line:" << timeLine;
       // qDebug() << "Text:" << text;

        while (!in.atEnd()) {
            QString t = in.readLine();
            if (t.trimmed().isEmpty())
                break;
            text += t + "\n";
             // qDebug() << "Text:" << text;
        }

        auto parts = timeLine.split(" --> ", Qt::SkipEmptyParts);
        if (parts.size() != 2)
            continue;

        QString start = parts[0].trimmed();
        QString end   = parts[1].trimmed();



        Subtitle sub;
        sub.startMs = timeToMs(start);
        sub.endMs   = timeToMs(end);
        sub.text    = text.trimmed();

       // qDebug() << start << sub.startMs << "->"
           //      << end   << sub.endMs;
        subtitles.append(sub);






    }
}

void MainWindow::updateSubtitle(qint64 positionMs)
{

  //  qDebug() << "positionMs =" << positionMs;

    if (subtitles.isEmpty()) {
        qDebug() << "subtitles empty";
        return;
    }

   // qDebug() << "first subtitle:"
   //          << subtitles.first().startMs
    //         << subtitles.first().endMs;



    static int lastIndex = -1;

    for (int i = 0; i < subtitles.size(); ++i) {
        const auto &sub = subtitles[i];
        if (positionMs >= sub.startMs && positionMs <= sub.endMs) {
            if (i != lastIndex) {
                ui->textEdit->setPlainText(sub.text);
                lastIndex = i;
            }
            return;
        }
    }

    // 該当なし
    ui->textEdit->clear();
    lastIndex = -1;
}

