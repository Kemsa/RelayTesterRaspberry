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
        HBridge_forward_all = 0x61,
        HBridge_reverse_all = 0x16,
        HBridge_forward_p1 = 0x21,
        HBridge_reverse_p1 = 0x12,
        HBridge_forward_p2 = 0x41,
        HBridge_reverse_p2 = 0x14,
    };

    static ContactSelector* initialize(int s0, int s1, int s2, int en,
                                       int hbridge1_top, int hbridge2_top, int hbridge3_top,
                                       int hbridge1_bottom, int hbridge2_bottom, int hbridge3_bottom);
    static ContactSelector* instance();
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
        hbridge1_top_mask = 0x01,
        hbridge2_top_mask = 0x02,
        hbridge3_top_mask = 0x04,
        hbridge1_bottom_mask = 0x10,
        hbridge2_bottom_mask = 0x20,
        hbridge3_bottom_mask = 0x40
    };

    ContactSelector(int s0, int s1, int s2, int en,
                    int hbridge1_top, int hbridge2_top, int hbridge3_top,
                    int hbridge1_bottom, int hbridge2_bottom, int hbridge3_bottom);

    static ContactSelector* s_instance;

    int m_currentContact = 0;
    int m_currentHBridgeOption = HBridge_none;
    const int m_s0;
    const int m_s1;
    const int m_s2;
    const int m_en;
    const int m_hbridge1_top;
    const int m_hbridge2_top;
    const int m_hbridge3_top;
    const int m_hbridge1_bottom;
    const int m_hbridge2_bottom;
    const int m_hbridge3_bottom;

    QMap<int, PinSelection> m_contactMap;
};

Q_DECLARE_METATYPE(ContactSelector::HBridge_options)

#endif // CONTACTSELECTOR_H
