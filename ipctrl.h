#ifndef IPCTRL_H
#define IPCTRL_H

#include <QFrame>
#include <QLineEdit>

class IPCtrl : public QFrame
{
    Q_OBJECT

public:
    IPCtrl(QWidget *parent = nullptr);
    ~IPCtrl();

    virtual bool eventFilter( QObject *obj, QEvent *event );

    QString text();
    bool isTextValid(const QString &strIP);
    bool setText(const QString &strIP);

public slots:
    void slotTextChanged( QLineEdit* pEdit );

signals:
    void signalTextChanged( QLineEdit* pEdit );
    void textChanged();

private:
    enum
    {
        QTUTL_IP_SIZE   = 4,// число октетов IP адресе
        MAX_DIGITS      = 3 // число символов в LineEdit
    };

    QLineEdit *(m_pLineEdit[QTUTL_IP_SIZE]);
    void MoveNextLineEdit (int i);
    void MovePrevLineEdit (int i);
};

#endif // IPCTRL_H
