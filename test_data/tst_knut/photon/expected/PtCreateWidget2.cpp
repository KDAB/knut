void bah() {
    PtSetArg(&args[0], Pt_ARG_AREA, &area, 0);
    PtSetArg(&args[2 + 2], Pt_ARG_METER_KEY_LEFT, Qt::Key_Down, 0);
    PtSetArg(&args[curArg], Pt_ARG_METER_KEY_RIGHT, Qt::Key_Up, 0);
    PtSetArg(&args[ignore], Pt_ARG_METER_INCREMENT, 10, 0);
    Photon::Meter *PhotonMeter1 = new Photon::Meter(parent);
    int curArg=    0;
    PhotonMeter1->setGeometry(&area);
    PhotonMeter1->setKeyLeft(Qt::Key_Down);
    PhotonMeter1->setKeyRight(Qt::Key_Up);
    PhotonMeter1->setIncrement(10);
    connect(PhotonMeter1, SIGNAL(KDAB_FIX_Pt_CB_METER_MOVED), &cb[0]) /*KDAB_NOTE: add connection for each member of callback array. Array len: 0*/;
    Photon::Meter *PhotonMeter2 = new Photon::Meter(parent);
    curArg   =    0  ;
    PhotonMeter2->setGeometry(&area);
    PhotonMeter2->setKeyLeft(Qt::Key_Down);
    PhotonMeter2->setKeyRight(Qt::Key_Up);
    PhotonMeter2->setIncrement(10);
    connect(PhotonMeter2, SIGNAL(KDAB_FIX_Pt_CB_METER_MOVED), &cb[0]) /*KDAB_NOTE: add connection for each member of callback array. Array len: 0*/;
}
