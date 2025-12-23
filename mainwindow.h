#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringListModel>
#include <QMediaPlayer>
#include <QAudioOutput>   // Qt6


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

struct Subtitle {
        qint64 startMs;
        qint64 endMs;
        QString text;
    };

private slots:
    void on_pushButton_clicked();
    void on_listView_doubleClicked(const QModelIndex &index);
    void saveLastPlayed(const QString &fileName);
    void restoreLastPlayed();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

private:
    Ui::MainWindow *ui;
    void saveSettings();
    void loadSettings();
    QStringListModel *mp3Model;
    void updateMp3List(const QString &dirPath);
    QMediaPlayer *player;
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    QAudioOutput *audioOutput;

    QVector<Subtitle> subtitles;
    void loadSrt(const QString &mp3Path);
    void updateSubtitle(qint64 positionMs);


   void rewindMs(qint64 ms);
   QString formatTime(qint64 ms);

   void searchSelectedWord();
   void setupContextMenu();
#endif
};
#endif // MAINWINDOW_H
