void bah() {
    PhArea_t area = { { 10, 10 }, { 200, 200 } };
    PtCallback_t cb[1] = { {moved_cb, NULL} };
    PtSetArg(&args[0], Pt_ARG_AREA, &area, 0);
    PtSetArg(&args[1], Pt_ARG_METER_KEY_LEFT, Pk_Down, 0);
    PtSetArg(&args[2], Pt_ARG_METER_KEY_RIGHT, Pk_Up, 0);
    PtSetArg(&args[3], Pt_ARG_METER_INCREMENT, 10, 0);
    PtSetArg(&args[4], Pt_CB_METER_MOVED, &cb[0], 0);
    PtCreateWidget( PtMeter, parent, 5, args );
}

void bah() {
    int nargs = 0;
    PtCreateWidget( CustomWgt, parent, nargs, 0 );
}

void bah() {
    int nargs = 0;
    cantFindWhereThisIsDeclared = PtCreateWidget( CustomWgt, parent, nargs, 0 );
    int innocentByStanderLineWhichShouldNotBeTouched = 0;
}

void bah() {
    int nargs = 0;
    PtWidget_t *widget = PtCreateWidget( PtButton, NULL, 0, NULL );
    PtWidget_t *widget = PtCreateWidget( PtLabel, 0, 0, 0 );
    PtWidget_t *widget = PtCreateWidget( PtLabel, Pt_NO_PARENT, 0, 0 );
}
