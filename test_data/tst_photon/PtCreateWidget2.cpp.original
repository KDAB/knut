void bah() {
    PtSetArg(&args[0], Pt_ARG_AREA, &area, 0);
    PtSetArg(&args[2 + 2], Pt_ARG_METER_KEY_LEFT, Pk_Down, 0);
    PtSetArg(&args[curArg], Pt_ARG_METER_KEY_RIGHT, Pk_Up, 0);
    PtSetArg(&args[ignore], Pt_ARG_METER_INCREMENT, 10, 0);
    int curArg=    0;
    PtSetArg(&args[curArg++], Pt_ARG_AREA, &area, 0);
    PtSetArg(&args[curArg++], Pt_ARG_METER_KEY_LEFT, Pk_Down, 0);
    PtSetArg(&args[curArg++], Pt_ARG_METER_KEY_RIGHT, Pk_Up, 0);
    PtSetArg(&args[curArg++], Pt_ARG_METER_INCREMENT, 10, 0);
    PtSetArg(&args[curArg++], Pt_CB_METER_MOVED, &cb[0], 0);
    PtCreateWidget( PtMeter, parent, curArg, args );
    curArg   =    0  ;
    PtSetArg(&args[curArg++], Pt_ARG_AREA, &area, 0);
    PtSetArg(&args[curArg++], Pt_ARG_METER_KEY_LEFT, Pk_Down, 0);
    PtSetArg(&args[curArg++], Pt_ARG_METER_KEY_RIGHT, Pk_Up, 0);
    PtSetArg(&args[curArg++], Pt_ARG_METER_INCREMENT, 10, 0);
    PtSetArg(&args[curArg++], Pt_CB_METER_MOVED, &cb[0], 0);
    PtCreateWidget( PtMeter, parent, curArg, args );
}
