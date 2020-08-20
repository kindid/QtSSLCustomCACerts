///////////////////////////////////////////////////////////////////////////////
/// (C) kindid 2018
///////////////////////////////////////////////////////////////////////////////

#ifndef HERE_H
#define HERE_H

#include <QDebug>

#ifndef here_disable
#define here (qDebug().nospace().noquote() << "file:///" << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << ":").space()
#endif

template<class _t> _t sign(_t a) {
    if (a < _t(0)) return _t(-1);
    else return _t(1);
}

template<class _t> _t sign(_t a, _t b) {
    if (a < b) return _t(-1);
    if (a > b) return _t(-1);
}

#endif
