#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qfile.h>
#include <QDebug>
#include <QWebFrame>
#include <QFileDialog>
#include <QSettings>
#include <QInputDialog>
#include <QColorDialog>

#include "analyser/analysermanager.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QSettings s;
    ui->startdir->setText(s.value(QLatin1String("startdir")).toString());
    m_analysemanager = new AnalyserManager();
    ui->lineFileFIlter->setText(m_analysemanager->getFileFilter().join(QLatin1String(",")));
    ui->btnAnalyse->setText(trUtf8("Analysieren"));
    connect(m_analysemanager, SIGNAL(progressValueChanged(int)), SLOT(progressValueChanged(int)));
    connect(m_analysemanager, SIGNAL(finishedCompletly(bool)), SLOT(finished(bool)));

    ui->listConcerns->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->listConcerns->addAction(ui->actionActionIFDEF);

    ui->stackedWidget->setCurrentIndex(0);
#ifndef WITHPDF
    ui->btnOutputPDF->setEnabled(false);
    ui->btnOutputPNG->setEnabled(false);
#endif
}

MainWindow::~MainWindow()
{
    delete m_analysemanager;
    delete ui;
}

void MainWindow::on_btnSearch_clicked()
{
  QString dir = QFileDialog::getExistingDirectory(this, trUtf8("Wähle Startverzeichnis"), ui->startdir->text());
  if (dir.isEmpty())
      return;
  ui->startdir->setText(dir);
  QSettings s;
  s.setValue(QLatin1String("startdir"), dir);
  ui->btnAnalyse->setEnabled(true);
}

void MainWindow::on_btnAnalyse_clicked()
{
    ui->btnRefresh->setEnabled(false);
    if (m_analysemanager->isRunning()) {
      m_analysemanager->cancel();
    } else {
      QSettings s;
      s.setValue(QLatin1String("startdir"), ui->startdir->text());
      m_analysemanager->setStartDirectory(QDir(ui->startdir->text()), ui->chkRecursiv->isChecked());
      m_analysemanager->setFileFilter(ui->lineFileFIlter->text().replace(QLatin1String(" "),QString()).split(QLatin1String(",")));
      m_analysemanager->start();
      ui->btnAnalyse->setText(trUtf8("Abbrechen"));
    }
}

void MainWindow::finished(bool aborted) {
  Q_UNUSED(aborted);
  ui->btnAnalyse->setText(trUtf8("Analysieren"));
  ui->btnSwitchToOutput->setEnabled(true);
}

void MainWindow::progressValueChanged(int v) {
  ui->lblProcessedFiles->setText(QString::number(v));
}

void MainWindow::on_btnSearchINI_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, trUtf8("Wähle Analysedatei"), ui->lineProjectFile->text(), trUtf8("Analysedatei (*.fileAnalyse)"));
    m_analysemanager->readINI(filename);
    ui->lineProjectFile->setText(filename);
}

void MainWindow::on_btnRefresh_clicked()
{
  QStringList l;
  for(int i=0,len=ui->listConcerns->count();i<len;++i) {
      m_analysemanager->setIFDEFEnabled(ui->listConcerns->item(i)->text(), (ui->listConcerns->item(i)->checkState()==Qt::Checked));
  }
  for(int i=0,len=ui->listFiles->count();i<len;++i) {
        m_analysemanager->setFileEnable(i, (ui->listFiles->item(i)->checkState()==Qt::Checked));
  }

  m_analysemanager->setVerticalOutput(ui->chkVertical->isChecked());
  m_analysemanager->setOutputWithLinebreak(ui->chkUmbruch->isChecked(), ui->spinUmbruch->value());
  m_analysemanager->setHideNotRelatedFiles(ui->chkHideFiles->isChecked());
  m_analysemanager->setFileSizeCorrelatesRectangle(ui->chkVariableSize);
  m_analysemanager->setFileItemDimensionLimit(ui->spinMin->value(), ui->spinMax->value(), ui->spinWidth->value());

  ui->webView->setContent(m_analysemanager->generateSVG(), QLatin1String("image/svg+xml"));
  ui->webView->page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOn);
  ui->webView->page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOn);
  ui->btnRefresh->setEnabled(false);
}

void MainWindow::on_btnToggle_clicked()
{
  for(int i=0,len=ui->listConcerns->count();i<len;++i) {
    ui->listConcerns->item(i)->setCheckState( ui->listConcerns->item(i)->checkState()==Qt::Checked?Qt::Unchecked:Qt::Checked);
  }
}

void MainWindow::on_btnAll_clicked()
{
  for(int i=0,len=ui->listConcerns->count();i<len;++i) {
    ui->listConcerns->item(i)->setCheckState(Qt::Checked);
  }
}

void MainWindow::on_btnFilter_clicked()
{
    QString text = QInputDialog::getText(this, tr("Filterbegriff"), tr("Es werden alle Elemente in der Liste ausgewählt, welche den angegeben Text enhalten."));
    if (text.isEmpty())
        return;
    for(int i=0,len=ui->listConcerns->count();i<len;++i) {
        if (ui->listConcerns->item(i)->text().contains(text))
            ui->listConcerns->item(i)->setCheckState(Qt::Checked);
    }
}

void MainWindow::on_btnToggle_2_clicked()
{
  for(int i=0,len=ui->listFiles->count();i<len;++i) {
    ui->listFiles->item(i)->setCheckState( ui->listFiles->item(i)->checkState()==Qt::Checked?Qt::Unchecked:Qt::Checked);
  }
}

void MainWindow::on_btnAll_2_clicked()
{
  for(int i=0,len=ui->listFiles->count();i<len;++i) {
    ui->listFiles->item(i)->setCheckState(Qt::Checked);
  }
}

void MainWindow::on_btnFilter_2_clicked()
{
    QString text = QInputDialog::getText(this, tr("Filterbegriff"), tr("Es werden alle Elemente in der Liste ausgewählt, welche den angegeben Text enhalten."));
    if (text.isEmpty())
        return;
    for(int i=0,len=ui->listFiles->count();i<len;++i) {
        if (ui->listFiles->item(i)->text().contains(text))
            ui->listFiles->item(i)->setCheckState(Qt::Checked);
    }
}

void MainWindow::on_btnSwitchToOutput_clicked()
{
    QList<AnalyserManager::ifdefStruct> l = m_analysemanager->ifdefs();
    for (int i=0;i< l.size();++i) {
        QListWidgetItem* item = new QListWidgetItem(l[i].name);
      item->setCheckState(l[i].enabled? Qt::Checked : Qt::Unchecked);
      setItemColor(item, l[i].color);
      ui->listConcerns->addItem(item);
    }
    QStringList files = m_analysemanager->foundFiles();
    for (int i=0;i< files.size();++i) {
        QListWidgetItem* item = new QListWidgetItem(files[i]);
        item->setCheckState(Qt::Checked);
        ui->listFiles->addItem(item);
    }
    on_btnRefresh_clicked();
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_btnOutputINI_clicked()
{
    QSettings s;
    QString filename = QFileDialog::getSaveFileName(this, tr("Wähle Analysedatei"),
                                                    s.value(QLatin1String("inifilename")).toString(), QLatin1String("Analysedatei (*.fileAnalyse)"));
    if (QFileInfo(filename).suffix().toLower()!=QLatin1String("fileanalyse"))
      filename += QLatin1String(".fileAnalyse");
    s.setValue(QLatin1String("inifilename"), filename);
    m_analysemanager->generateINI(filename);
}

void MainWindow::on_btnOutputSVG_clicked()
{
    QSettings s;
    QString filename = QFileDialog::getSaveFileName(this, tr("Wähle Dateinamen"),
                                                    s.value(QLatin1String("svgfilename")).toString(), QLatin1String("SVG (*.svg)"));
    if (QFileInfo(filename).suffix().toLower()!=QLatin1String("svg"))
      filename += QLatin1String(".svg");
    s.setValue(QLatin1String("svgfilename"), filename);
    m_analysemanager->generateSVG(filename);
}

#ifdef WITHPDF
#define PIXELS_PER_POINT 1
#include <librsvg/rsvg.h>
#include <cairo-pdf.h>

void saveWithCairo(bool usepng, const unsigned char* svgdatabuf, int svgbuflen, const char* output_filename) {
    GError *error = NULL;
    RsvgHandle *handle;
    RsvgDimensionData dim;
    double width, height;
    cairo_surface_t *surface;
    cairo_t *cr;
    cairo_status_t status;

    g_type_init ();

    rsvg_set_default_dpi (72.0);
    handle = rsvg_handle_new_from_data (svgdatabuf, svgbuflen, &error);
    if (error != NULL) {
        qDebug () << error->message;
        return;
    }

    rsvg_handle_get_dimensions (handle, &dim);
    width = dim.width;
    height = dim.height;

    if (usepng) {
        surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
    }
    else {
        surface = cairo_pdf_surface_create (output_filename, width, height);
    }

    cr = cairo_create (surface);

    rsvg_handle_render_cairo (handle, cr);

    status = cairo_status (cr);
    if (status) {
        qDebug () << cairo_status_to_string (status);
        return;
    }

    if (usepng) {
        cairo_surface_write_to_png (surface, output_filename);
    }

    cairo_destroy (cr);
    cairo_surface_destroy (surface);
}
#else
void saveWithCairo(bool usepng, const unsigned char* svgdatabuf, int svgbuflen, const char* output_filename) {
}
#endif

void MainWindow::on_btnOutputPDF_clicked()
{
    QSettings s;
    QString filename = QFileDialog::getSaveFileName(this, tr("Wähle Dateinamen"),
                                                    s.value(QLatin1String("pdffilename")).toString(), QLatin1String("PDF (*.pdf)"));
    if (QFileInfo(filename).suffix().toLower()!=QLatin1String("pdf"))
      filename += QLatin1String(".pdf");
    s.setValue(QLatin1String("pdffilename"), filename);
    const QByteArray svgdata = m_analysemanager->generateSVG();
    const QByteArray output_filename_bytearray = filename.toUtf8();
    saveWithCairo(false, (const unsigned char*)svgdata.constData(), svgdata.length(), output_filename_bytearray.data());
}

void MainWindow::on_btnOutputPNG_clicked()
{
    QSettings s;
    QString filename = QFileDialog::getSaveFileName(this, tr("Wähle Dateinamen"),
                                                    s.value(QLatin1String("pdffilename")).toString(), QLatin1String("PDF (*.pdf)"));
    if (QFileInfo(filename).suffix().toLower()!=QLatin1String("pdf"))
      filename += QLatin1String(".pdf");
    s.setValue(QLatin1String("pdffilename"), filename);
    const QByteArray svgdata = m_analysemanager->generateSVG();
    const QByteArray output_filename_bytearray = filename.toUtf8();
    saveWithCairo(false, (const unsigned char*)svgdata.constData(), svgdata.length(), output_filename_bytearray.data());
}


void MainWindow::on_listConcerns_itemChanged(QListWidgetItem *)
{
  ui->btnRefresh->setEnabled(true);
}

void MainWindow::on_listFiles_itemChanged(QListWidgetItem *)
{
    ui->btnRefresh->setEnabled(true);
}

void MainWindow::on_chkVertical_stateChanged(int)
{
    ui->btnRefresh->setEnabled(true);
}

void MainWindow::on_chkUmbruch_stateChanged(int)
{
    ui->btnRefresh->setEnabled(true);
}

void MainWindow::on_chkVariableSize_stateChanged(int)
{
    ui->btnRefresh->setEnabled(true);
}

void MainWindow::on_chkHideFiles_stateChanged(int)
{
    ui->btnRefresh->setEnabled(true);
}

void MainWindow::on_spinUmbruch_valueChanged(int)
{
    ui->btnRefresh->setEnabled(true);
}

void MainWindow::on_spinMax_valueChanged(int)
{
    ui->btnRefresh->setEnabled(true);
}

void MainWindow::on_spinMin_valueChanged(int)
{
    ui->btnRefresh->setEnabled(true);
}

void MainWindow::on_spinWidth_valueChanged(int)
{
    ui->btnRefresh->setEnabled(true);
}

void MainWindow::on_btnSelectBgColor_clicked()
{
    QColorDialog d;
    d.setCurrentColor(m_analysemanager->fileBackgroundColor());
    d.setOption(QColorDialog::ShowAlphaChannel, true);
    if (d.exec() ==QDialog::Accepted) {
        ui->btnRefresh->setEnabled(true);
        m_analysemanager->setFileBackgroundColor(d.currentColor());
    }
}

void MainWindow::setItemColor(QListWidgetItem*item, QColor color) {
    item->setBackgroundColor(color);
    if (color.lightness()<186*color.alphaF())
        item->setForeground(QColor(255,255,255));
    else
        item->setForeground(QColor(0,0,0));
    m_analysemanager->setIFDEFColor(item->text(), color);
}

void MainWindow::on_actionActionIFDEF_triggered()
{
    QListWidgetItem* temp = ui->listConcerns->currentItem(); //ui->listConcerns->itemAt(ui->listConcerns->mapFromGlobal(QCursor::pos()));
    if(temp == NULL)
        return;
    QColorDialog d;
    d.setWindowTitle(trUtf8("Farbe wählen für ")+temp->text());
    d.setCurrentColor(m_analysemanager->IFDEFColor(temp->text()));
    d.setOption(QColorDialog::ShowAlphaChannel, true);
    if (d.exec() ==QDialog::Accepted) {
        setItemColor(temp, d.currentColor());
    }
}

void MainWindow::on_btnSelectFgColor_clicked()
{
    QColorDialog d;
    d.setCurrentColor(m_analysemanager->fileMarkDefaultColor());
    d.setOption(QColorDialog::ShowAlphaChannel, true);
    if (d.exec() ==QDialog::Accepted) {
        ui->btnRefresh->setEnabled(true);
        for (int i=0;i< ui->listConcerns->count();++i) {
            QListWidgetItem* item = ui->listConcerns->item(i);
            setItemColor(item, d.currentColor());
        }
    }
}
