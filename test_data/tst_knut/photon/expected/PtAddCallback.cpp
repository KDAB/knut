int ptconnect()
{
    QObject::connect(ABW_CLstKvInstTRAJ, itemSelectionChanged(), cbSimpleClicLstFormatTraj);
    QObject::connect(ABW_CValeurWrdTRAJ, &Photon::Text::modifyNotify, cbModifCziTraj);
    QObject::connect(ABW_CValeurWrdTRAJ, &Photon::Text::modifyNotify, cbModifCziTraj);
    QObject::connect(ABW_CBinstWaitComplement, &Photon::Button::activate, cbWaitSetTypeTest);
    QObject::connect(ABW_CBtestPopup, &Photon::Button::arm, cbAppuiTestPopup);
    QObject::connect(ABW_CBtestPopup, &Photon::Button::disarm, cbRelacherTestPopup);
    QObject::connect(ABW_DtimerCleSecu, timeout(), cb2TimerCleSecu);
    QObject::connect(ABW_DtimerCleSecu, timeout(), cb2TimerCleSecu)/*KDAB_NOTE: Callback data was someData*/;
}
