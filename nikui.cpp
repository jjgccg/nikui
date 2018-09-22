#include <QtWidgets>

#include "nikui.h"
#include "ui_nikui.h"

static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode);

Nikui::Nikui() :
    imageLabel(new QLabel),
    scrollArea(new QScrollArea)
{
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    scrollArea->setVisible(false);
    setCentralWidget(scrollArea);

    createActions();

    resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);
}

Nikui::~Nikui() {}

/* Fill in menu and associate actions with buttons */
void Nikui::createActions()
{    
    /* File Menu */
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    QAction *openAct = fileMenu->addAction(tr("&Open..."), this, &Nikui::open);
    openAct->setShortcut(QKeySequence::Open);

    fileMenu->addSeparator();

    QAction *exitAct = fileMenu->addAction(tr("E&xit"), this, &QWidget::close);
    exitAct->setShortcut(tr("Ctrl+Q"));

    /* Edit Menu */

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));

    /* View Menu */

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));

    prevPageAct = viewMenu->addAction(tr("&Previous Page"), this, &Nikui::prevPage);
    prevPageAct->setShortcut(tr("Left")); // <- goes to previous

    nextPageAct = viewMenu->addAction(tr("&Next Page"), this, &Nikui::nextPage);
    nextPageAct->setShortcut(tr("Right")); // -> goes to next

    viewMenu->addSeparator();

    zoomInAct = viewMenu->addAction(tr("Zoom &In (25%)"), this, &Nikui::zoomIn);
    zoomInAct->setShortcut(QKeySequence::ZoomIn);
    zoomInAct->setEnabled(false);

    zoomOutAct = viewMenu->addAction(tr("Zoom &Out (25%)"), this, &Nikui::zoomOut);
    zoomOutAct->setShortcut(QKeySequence::ZoomOut);
    zoomOutAct->setEnabled(false);

    normalSizeAct = viewMenu->addAction(tr("&Normal Size"), this, &Nikui::normalSize);
    normalSizeAct->setShortcut(tr("Ctrl+S"));
    normalSizeAct->setEnabled(false);

    viewMenu->addSeparator();

    fitToWindowAct = viewMenu->addAction(tr("&Fit to Window"), this, &Nikui::fitToWindow);
    fitToWindowAct->setEnabled(false);
    fitToWindowAct->setCheckable(true);
    fitToWindowAct->setShortcut(tr("Ctrl+F"));

    /* Help Menu */

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    helpMenu->addAction(tr("&About"), this, &Nikui::about);
}

/* Store all user selected files for access */
bool Nikui::loadFiles(const QStringList& fileNames)
{
    QImage newImage;
    for(int file = 0; file < fileNames.size(); file++)
    {
        QImageReader reader(fileNames.at(file));
        newImage = reader.read();
        if(newImage.isNull()) return false;

        currentMangaFileList.push_back(newImage);
    }

    initializeView(); // set position in list to 1 and display first
                      // image from the list

    return true;
}

/* Initialize the window by displaying the first file */
void Nikui::initializeView()
{
    currentMangaFilePos = 0;
    setImage(currentMangaFileList.at(0));
}

/* Called when user navigates back and forth between images */
void Nikui::setImage(const QImage& newImage)
{
    currentImage = newImage;
    imageLabel->setPixmap(QPixmap::fromImage(currentImage));
    scaleFactor = 1.0;

    scrollArea->setVisible(true);
    fitToWindowAct->setEnabled(true);
    updateActions();

    if (!fitToWindowAct->isChecked()) imageLabel->adjustSize();
}

void Nikui::prevPage()
{
    if(currentMangaFilePos > 0)
    {
        currentMangaFilePos--;
        setImage(currentMangaFileList.at(currentMangaFilePos));
    }
}

void Nikui::nextPage()
{
    if(currentMangaFilePos < currentMangaFileList.size() - 1)
    {
        currentMangaFilePos++;
        setImage(currentMangaFileList.at(currentMangaFilePos));
    }
}

void Nikui::open()
{
    QFileDialog dialog(this, tr("Open File"));

    const QStringList mangaLocation = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    dialog.setDirectory(mangaLocation.isEmpty() ? QDir::currentPath() : mangaLocation.last());
    QStringList fileList = dialog.getOpenFileNames(this, tr("JPG files"),QDir::currentPath(), tr("Jpeg files (*.jpg);;All files (*.*)"));

    loadFiles(fileList);
}

void Nikui::zoomIn()
{
    scaleImage(1.25);
}

void Nikui::zoomOut()
{
    scaleImage(0.8);
}

void Nikui::normalSize()
{
    imageLabel->adjustSize();
    scaleFactor = 1.0;
}

void Nikui::fitToWindow()
{
    bool fitToWindow = fitToWindowAct->isChecked();
    scrollArea->setWidgetResizable(fitToWindow);
    if (!fitToWindow) normalSize();

    updateActions();
}

void Nikui::scaleImage(double factor)
{
    Q_ASSERT(imageLabel->pixmap());
    scaleFactor *= factor;
    imageLabel->resize(scaleFactor * imageLabel->pixmap()->size());

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    zoomInAct->setEnabled(scaleFactor < 3.0);
    zoomOutAct->setEnabled(scaleFactor > 0.333);
}

void Nikui::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}

void Nikui::about()
{
    QMessageBox::about(this, tr("About nikui"),
            tr("<p>I created nikui because I wanted a simple, lightweight "
               "manga viewer that is not bogged down with too many features "
               "and prioritizes simplicity and ease of use.  I hope you "
               "enjoy the experience.</p><p>Please contact me for questions, "
               "comments, bugs, and yes, even manga recommendations, "
               "at jge9@protonmail.ch </p>"));
}

void Nikui::updateActions()
{
    zoomInAct->setEnabled(!fitToWindowAct->isChecked());
    zoomOutAct->setEnabled(!fitToWindowAct->isChecked());
    normalSizeAct->setEnabled(!fitToWindowAct->isChecked());
}