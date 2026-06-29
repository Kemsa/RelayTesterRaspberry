#ifndef CONTACTSELECTOR_H
#define CONTACTSELECTOR_H

#include <QMap>
#include <QObject>

class ContactSelector : public QObject {
    Q_OBJECT

public:
    static ContactSelector& initialize(int s1, int s2, int s3, int en);
    static ContactSelector& instance();

    ContactSelector(const ContactSelector&) = delete;
    ContactSelector& operator=(const ContactSelector&) = delete;
    ContactSelector(ContactSelector&&) = delete;
    ContactSelector& operator=(ContactSelector&&) = delete;

    int currentContact() { return m_currentContact; };

public slots:
    void selectContact(int contactIndex);
signals:
    void contactSelected(const int contactIndex);

private:
    enum PinSelection {
        // bit assign: s0, s1, s2
        Contact1 = 0 << 1 | 1,
        Contact2 = 1 << 1 | 1,
        Contact3 = 2 << 1 | 1,
        Contact4 = 3 << 1 | 1,
        Contact5 = 4 << 1 | 1,
        Contact6 = 5 << 1 | 1,
        Contact7 = 6 << 1 | 1,
        Contact8 = 7 << 1 | 1,
        None = 0 << 1 | 0
    };

    enum masks {
        s0_mask = 0x02,
        s1_mask = 0x04,
        s2_mask = 0x08,
        enable_mask = 0x01
    };

    ContactSelector(int s0, int s1, int s2, int en);

    static ContactSelector* s_instance;

    int m_currentContact = 0;
    const int m_s0;
    const int m_s1;
    const int m_s2;
    const int m_en;

    QMap<int, PinSelection> m_contactMap;
};

#endif // CONTACTSELECTOR_H
