#include "common.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScreen>
#include <QScrollArea>
#include <QTimer>
#include <memory>

int main(int argc, char **argv) {
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Liss Qt Labs"));
    QApplication::setApplicationVersion(QStringLiteral("1.0.0"));
    QApplication::setOrganizationName(QStringLiteral("Liss"));
    liss::applyTheme();
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Liss Qt Labs · C++ / Qt Widgets 桌面实验"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({QStringLiteral("showcase"), QStringLiteral("载入可继续操作的确定性示例场景")});
    parser.addOption({QStringLiteral("scenario"), QStringLiteral("示例变体"), QStringLiteral("name")});
    parser.addOption({QStringLiteral("capture"), QStringLiteral("保存真实 QWidget 渲染截图并退出"),
                      QStringLiteral("png")});
    parser.addOption(
        {QStringLiteral("report"), QStringLiteral("保存截图尺寸和运行信息"), QStringLiteral("json")});
    parser.addOption({QStringLiteral("width"), QStringLiteral("窗口逻辑宽度"), QStringLiteral("pixels"),
                      QStringLiteral("1400")});
    parser.addOption({QStringLiteral("height"), QStringLiteral("窗口逻辑高度"), QStringLiteral("pixels"),
                      QStringLiteral("1000")});
    parser.process(app);
    std::unique_ptr<QMainWindow> window;
#if defined(LAB_APP_OsLab)
    window.reset(liss::createOsWindow(parser.isSet(QStringLiteral("showcase")),
                                      parser.value(QStringLiteral("scenario"))));
#elif defined(LAB_APP_HuffmanLab)
    window.reset(liss::createHuffmanWindow(parser.isSet(QStringLiteral("showcase")),
                                           parser.value(QStringLiteral("scenario"))));
#else
    window.reset(liss::createCalcWindow(parser.isSet(QStringLiteral("showcase")),
                                        parser.value(QStringLiteral("scenario"))));
#endif
    // Preserve content sizes on small laptop screens; scroll instead of shrinking
    // input fields or letting controls overlap their panels.
    QWidget *content = window->takeCentralWidget();
    for (QFrame *frame : content->findChildren<QFrame *>(QStringLiteral("panel"))) {
        if (frame->layout())
            frame->layout()->activate();
        frame->setMinimumSize(frame->minimumSizeHint());
    }
    if (content->layout())
        content->layout()->activate();
    content->setMinimumSize(content->minimumSizeHint());
    auto *scroll = new QScrollArea;
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidgetResizable(true);
    scroll->setWidget(content);
    window->setCentralWidget(scroll);
    bool widthOkay = false, heightOkay = false;
    int width = parser.value(QStringLiteral("width")).toInt(&widthOkay),
        height = parser.value(QStringLiteral("height")).toInt(&heightOkay);
    if (widthOkay && heightOkay && width >= 1080 && width <= 3840 && height >= 760 && height <= 2160)
        window->resize(width, height);
    window->show();
    if (parser.isSet(QStringLiteral("capture"))) {
        QTimer::singleShot(650, &app, [&] {
            const QString path = QFileInfo(parser.value(QStringLiteral("capture"))).absoluteFilePath();
            QDir().mkpath(QFileInfo(path).absolutePath());
            QPixmap capture = window->grab();
            bool saved = capture.save(path);
            if (parser.isSet(QStringLiteral("report"))) {
                QFile report(parser.value(QStringLiteral("report")));
                if (report.open(QIODevice::WriteOnly)) {
                    QJsonObject data{{QStringLiteral("saved"), saved},
                                     {QStringLiteral("title"), window->windowTitle()},
                                     {QStringLiteral("qt"), QString::fromLatin1(qVersion())},
                                     {QStringLiteral("logicalWidth"), window->width()},
                                     {QStringLiteral("logicalHeight"), window->height()},
                                     {QStringLiteral("imageWidth"), capture.width()},
                                     {QStringLiteral("imageHeight"), capture.height()},
                                     {QStringLiteral("devicePixelRatio"), capture.devicePixelRatio()},
                                     {QStringLiteral("captureMethod"),
                                      QStringLiteral("QWidget::grab - actual native Qt widget tree")},
                                     {QStringLiteral("path"), path}};
                    report.write(QJsonDocument(data).toJson());
                }
            }
            app.exit(saved ? 0 : 2);
        });
    }
    return app.exec();
}
