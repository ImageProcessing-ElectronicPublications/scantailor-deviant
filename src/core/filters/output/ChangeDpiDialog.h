/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C)  Joseph Artsimovich <joseph.artsimovich@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OUTPUT_CHANGEDPIDIALOG_H_
#define OUTPUT_CHANGEDPIDIALOG_H_

#include "ui/ui_OutputChangeDpiDialog.h"
#include <QDialog>
#include <QString>

class Dpi;

namespace output
{

class ChangeDpiDialog : public QDialog, public Ui_OutputChangeDpiDialog
{
    Q_OBJECT
public:
    ChangeDpiDialog(QWidget* parent, Dpi const& dpi);

    int dpi() const
    {
        return dpiSelector->currentText().toInt();
    }
public slots:
    virtual void accept();
private slots:
    void dpiSelectionChanged(int index);

    void dpiEditTextChanged(QString const& text);
private:
    bool validate();

    int m_customItemIdx;
    QString m_customDpiString;
};

} // namespace output

#endif
