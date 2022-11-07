#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGridLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QDebug>
#include <QDialog>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QGridLayout *g = new QGridLayout(this->centralWidget());
    g -> addWidget(ui -> selectSingleFileButton, 0, 0, 1, 1);
    g -> addWidget(ui -> selectMultipleFilesButton, 0, 1, 1, 1);
    g -> addWidget(ui -> selectFolderButton, 0, 2, 1, 1);
    g -> addWidget(ui -> clearButton, 0, 3, 1, 1);
    g -> addWidget(ui -> dataHolder, 1, 0, 4, 4);
    ui -> dataHolder -> setColumnCount(5);
    ui -> dataHolder -> setHorizontalHeaderLabels({"имя", "размер", "разрешение", "глубина цвета", "сжатие"});
    ui -> dataHolder -> horizontalHeader() -> setSectionResizeMode(QHeaderView::Stretch);
    statusBar()->showMessage("Ничего не выбрано");
    setWindowTitle("Характеристики изображений");
    dirFilter << "*.jpg" << "*.gif" << "*.tif" << "*.bmp" << "*.png" << "*.pcx";
    compression["bmp"] = "без сжатия";
    compression["gif"] = "LZW сжатие";
    compression["png"] = "Deflate сжатие";
    compression["jpg"] = "JPEG сжатие";
    compression["tiff"] = "ZIP/LZW/JPEG сжатие";
    compression["pcx"] = "RLE сжатие";
    connect(ui -> dataHolder, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(tableItemClicked(int,int)));
}

void MainWindow::tableItemClicked(int row, int column)
{
    for (int i = 0; i < ui->dataHolder->columnCount(); ++i)
    {
        ui->dataHolder-> item(row, i) -> setSelected(false);
    }
    QDialog *d = new QDialog(this);
    QHBoxLayout diagLayout(d);
    QPixmap px(fileNames[ui->dataHolder-> item(row, 0) -> text()]);
    QLabel label;
    label.setPixmap(px);
    diagLayout.addWidget(&label);
    d->exec();

}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_selectFolderButton_clicked()
{
    QDir newDir(QFileDialog::getExistingDirectory(this, "Выберите папку", lastDir));
    if (!newDir.isEmpty())
    {
        newDir.setNameFilters(dirFilter);
        QFileInfoList fileList(newDir.entryInfoList(QDir::Filter::Files));
        QStringList filepaths;
        for (const auto& x : fileList)
        {
            filepaths.append(x.absoluteFilePath());
        }
        AppendDataIntoTable(filepaths);
        statusBar()->showMessage("Выбрана папка: " + newDir.dirName() + ", добавлено " + QString::number(filepaths.size()) + " файлов");
    }
}

void MainWindow::on_selectSingleFileButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Выбрать изображение", lastDir, "Выбрать изображения (*.jpg *.gif *.tif *.bmp *.png *.pcx)");
    if (!fileName.isNull())
    {
        AppendDataIntoTable({fileName});
        statusBar()->showMessage("Добавлен 1 файл", messageTimeout);
    }
}

void MainWindow::on_selectMultipleFilesButton_clicked()
{
    QStringList newFileNames = QFileDialog::getOpenFileNames(this, "Выбрать изображения", lastDir, "Выбрать изображения (*.jpg *.gif *.tif *.bmp *.png *.pcx)");
    if (!newFileNames.isEmpty())
    {
         AppendDataIntoTable(newFileNames);
         statusBar()->showMessage("Добавлено " + QString::number(newFileNames.size()) + " файлов", messageTimeout);
    }
}
void MainWindow::AppendDataIntoTable(const QStringList &list)
{
    int pos = ui -> dataHolder -> rowCount();
    ui -> dataHolder -> setRowCount(pos + list.size());
    for (int i = 0; i < list.size(); ++i)
    {
        QImage image(list[i]);
        if (!image.isNull())
        {
            QFileInfo info(list[i]);
            fileNames[info.fileName()] = list[i];
            lastDir = info.absolutePath();
            ui -> dataHolder -> setItem(pos + i, 0, new QTableWidgetItem(info.fileName()));
            ui -> dataHolder -> setItem(pos + i, 1, new QTableWidgetItem(QString::number(image.size().width()) + " X " + QString::number(image.size().height())));
            ui -> dataHolder -> setItem(pos + i, 2, new QTableWidgetItem(QString::number(static_cast<int>(std::min(image.dotsPerMeterX(), image.dotsPerMeterY()) / 39.37))));
            ui -> dataHolder -> setItem(pos + i, 3, new QTableWidgetItem(QString::number(image.depth())));
            ui -> dataHolder -> setItem(pos + i, 4, new QTableWidgetItem(compression[info.suffix()]));
        }
    }
}
void MainWindow::on_clearButton_clicked()
{
    QList<QTableWidgetItem*> selectedItems = ui -> dataHolder -> selectedItems();
    if (selectedItems.size() == 0)
    {
        if (ui -> dataHolder -> rowCount() != 0)
        {
            statusBar() -> showMessage("Удалено " + QString::number(ui -> dataHolder -> rowCount()) + " файлов", messageTimeout);
            ui -> dataHolder -> clearContents();
            ui -> dataHolder -> setRowCount(0);
            fileNames.clear();
        }
        else
        {
            statusBar() -> showMessage("Нечего удалять", messageTimeout);
        }
    }
    else
    {
        for (int i = selectedItems.size() / 4 - 1; i >= 0; --i)
        {
            ui -> dataHolder -> removeRow(selectedItems[4 * i]->row());
        }
        statusBar() -> showMessage("Удалено " + QString::number(selectedItems.size() / 4) + " файлов", messageTimeout);
    }
}

