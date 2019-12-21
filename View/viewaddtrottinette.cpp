#include "viewaddtrottinette.h"
#include "ui_viewaddtrottinette.h"
#include "Core/databasemanager.h"
#include "Core/utilisateur.h"
#include <QFileDialog>
#include "Core/applicationmanager.h"
#include <QDebug>
#include <QMessageBox>
#include "View/viewlistetrottinette.h"

ViewAddTrottinette::ViewAddTrottinette(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ViewAddTrottinette)
{
    ui->setupUi(this);

    connect(ui->choisirPhoto, &QPushButton::clicked, [this](){
        m_ImageFilePath = QFileDialog::getOpenFileName(this,
               tr("Choose Image"), "", tr("Image Files (*.png *.jpg *.bmp)"));
    });
}

bool ViewAddTrottinette::VerifyData()
{
    if (ApplicationManager::GetInstance().GetCurrentUser() == NULL){
        ui->warning->setText("Cette fonction n'est pas disponible lorsque vous n'êtes pas connecté.");
        return false;
    }

    QString ref = ui->ref->text();
    QSqlQuery& res = DatabaseManager::GetInstance().Exec("SELECT * FROM trottinettes WHERE ref_trotti = '%s'", ref.toLocal8Bit().constData());

    if (res.next()){ // Il ya des resultats alors return false et print l'error
        ui->warning->setText("La référence de la trottinette existe déjà \ndans la base de données.");
        return false;
    }

    return true;
}

void ViewAddTrottinette::SubmitData()
{
    QString ref = ui->ref->text();
    QString model = ui->model->text();
    QString etat = ui->etat->currentText();

    if (!m_ImageFilePath.isEmpty() && m_ImageFilePath != ""){
        QFile img(m_ImageFilePath);

        if (!img.open(QIODevice::ReadOnly)) {
            qDebug() << "Cant open file!";
            return;
        }
        QByteArray buffer_byte_array = img.readAll();
        /*QImage trotti_pic;
        trotti_pic.load(m_ImageFilePath);
        QByteArray buffer_byte_array = QByteArray::fromRawData((const char*)trotti_pic.bits(), trotti_pic.byteCount());*/

        DatabaseManager::GetInstance().
            Prepare(
                    "INSERT INTO trottinettes (ref_trotti, model, etat, image, identifiant)"
                    "VALUES ('%s', '%s', '%s', :imageData, '%s');",
                    ref.toLocal8Bit().constData(),
                    model.toLocal8Bit().constData(),
                    etat.toLocal8Bit().constData(),
                    ApplicationManager::GetInstance().GetCurrentUser()->GetIdentifiant().toLocal8Bit().constData()
            );
        DatabaseManager::GetInstance().BindValue(":imageData", buffer_byte_array);
        DatabaseManager::GetInstance().Exec();
    }else{
        DatabaseManager::GetInstance().
            Exec(
                    "INSERT INTO trottinettes (ref_trotti, model, etat, image, identifiant)"
                    "VALUES ('%s', '%s', '%s', NULL, '%s');",
                    ref.toLocal8Bit().constData(),
                    model.toLocal8Bit().constData(),
                    etat.toLocal8Bit().constData(),
                    ApplicationManager::GetInstance().GetCurrentUser()->GetIdentifiant().toLocal8Bit().constData()
            );
    }

    // Petite fenêtre pour informer l'utilisateur que tout va bien
    QMessageBox::information(
        this,
        tr("AJouter Une Trottinette"),
        QString("Votre trotinette a été bien ajouté dans notre base de donées.")
    );

    ((ViewListeTrottinette*)parent())->RefreshList();
    QDialog::accept(); // fermer la fenêtre de dialog
}


void ViewAddTrottinette::accept()
{
    if (this->VerifyData()){
        this->SubmitData();
    }
}

ViewAddTrottinette::~ViewAddTrottinette()
{
    delete ui;
}