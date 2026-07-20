#ifndef CONTACTSELECTOR_H
#define CONTACTSELECTOR_H

#include <QMap>
#include <QObject>
#include <QString>

class ContactSelector : public QObject {
    Q_OBJECT

public:
    enum HBridge_options {
        HBridge_none = 0x00,
        HBridge_forward_all = 0x06,
        HBridge_reverse_all = 0x01,
        HBridge_forward_p1 = 0x02,
        HBridge_reverse_p1 = 0x05,
        HBridge_forward_p2 = 0x04,
        HBridge_reverse_p2 = 0x03,
    };

    static ContactSelector& initialize(int s0, int s1, int s2, int en,
                                       int hbridge1, int hbridge2, int hbridge3);
    static ContactSelector& instance();
    static QString hBridgeOptionToString(HBridge_options option);

    ContactSelector(const ContactSelector&) = delete;
    ContactSelector& operator=(const ContactSelector&) = delete;
    ContactSelector(ContactSelector&&) = delete;
    ContactSelector& operator=(ContactSelector&&) = delete;

    int currentContact() { return m_currentContact; };

public slots:
    void selectContact(int contactIndex);
    void selectHBridge(HBridge_options option);
signals:
    void contactSelected(const int contactIndex);
    void hBridgeOptionSelected(const HBridge_options option);

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

    enum select_masks {
        s0_mask = 0x02,
        s1_mask = 0x04,
        s2_mask = 0x08,
        enable_mask = 0x01
    };

    enum HBridge_masks {
        hbridge1_mask = 0x01,
        hbridge2_mask = 0x02,
        hbridge3_mask = 0x04
    };

    ContactSelector(int s0, int s1, int s2, int en,
                    int hbridge1, int hbridge2, int hbridge3);

    static ContactSelector* s_instance;

    int m_currentContact = 0;
    int m_currentHBridgeOption = HBridge_none;
    const int m_s0;
    const int m_s1;
    const int m_s2;
    const int m_en;
    const int m_hbridge1;
    const int m_hbridge2;
    const int m_hbridge3;

    QMap<int, PinSelection> m_contactMap;
};

Q_DECLARE_METATYPE(ContactSelector::HBridge_options)

#endif // CONTACTSELECTOR_H
