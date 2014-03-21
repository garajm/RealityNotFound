#ifndef CAMSELECTWINDOW_H
#define CAMSELECTWINDOW_H

#include <QDialog>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QComboBox>
#include <QtGui/QTableWidget>
#include <QtGui/QTableWidgetItem>
#include <QtGui/QStandardItemModel>
#include <QtGui/QTreeView>
#include <QtGui/QTableView>
#include <OpenCV/CamSelectCore.h>

namespace OpenCV
{
/**
	 * @author Autor: Marek Jakab
	 * @brief Class CamSelectWindow
	 */
class CamSelectWindow : public QDialog
{
	Q_OBJECT

public:
	/**
		 * @author Autor: Marek Jakab
		 * @brief CamSelectWindow Constructor
		 * @param parent Parent Widget
		 * @param app QApplication
		 */
	CamSelectWindow(QWidget *parent, QApplication *app, QString stringdata);
	QLabel *getLabel();
	QLabel *windowLabel;
signals:
	void setCam(int devid, int width, int heigth);
private:
	QApplication * app;

	QStandardItemModel * model;
	QTreeView * view;
	QList<QString> * changes;
	QPushButton * applyButton;
	QModelIndexList indexes;

	int rowsize;
	/**
		 * @author Autor: Marek Jakab
		 * @brief configureWindow Sets window properties
		 */
	void configureWindow(QString stringdata);
	void createTableModel(QString stringdata);

public slots:
	//void itemClicked(QModelIndex index);
	void commitChanges();
};
}

#endif // CAMSELECTWINDOW_H