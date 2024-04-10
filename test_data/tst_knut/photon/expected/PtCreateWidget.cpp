void bah() {
    QRect area = { { 10, 10 }, { 200, 200 } };
    PtCallback_t cb[1] = { {moved_cb, NULL} };
    Photon::Meter *PhotonMeter1 = new Photon::Meter(parent);
    PhotonMeter1->setGeometry(&area);
    PhotonMeter1->setKeyLeft(Qt::Key_Down);
    PhotonMeter1->setKeyRight(Qt::Key_Up);
    PhotonMeter1->setIncrement(10);
    connect(PhotonMeter1, SIGNAL(KDAB_FIX_Pt_CB_METER_MOVED), &cb[0]) /*KDAB_NOTE: add connection for each member of callback array. Array len: 0*/;
}

void bah() {
    KDAB_FIX_UNKNOWN_CLASS_CustomWgt *KDAB_FIX_UNKNOWN_CLASS_CustomWgt1 = new KDAB_FIX_UNKNOWN_CLASS_CustomWgt(parent);
    int nargs = 0;
}

void bah() {
    cantFindWhereThisIsDeclared = new KDAB_FIX_UNKNOWN_CLASS_CustomWgt(parent);
    int nargs = 0;
    int innocentByStanderLineWhichShouldNotBeTouched = 0;
}

void bah() {
    int nargs = 0;
    Photon::Button *widget = new Photon::Button(NULL);
    Photon::Label *widget = new Photon::Label(0);
    Photon::Label *widget = new Photon::Label(0);
}
