void bah() {
    PtArg_t args[3];
    PtWidget_t *widget;
    nargs = 0;
    if (true) {
        PtSetArg(&args[nargs++], Pt_ARG_AREA, &area, 0);
        PtSetArg(&args[nargs++], Pt_ARG_METER_KEY_LEFT, Pk_Down, 0);
        PtSetArg(&args[nargs++], Pt_ARG_METER_KEY_RIGHT, Pk_Up, 0);
    } else {
        PtSetArg(&args[nargs++], Pt_ARG_METER_INCREMENT, 10, 0);
        PtSetArg(&args[nargs++], Pt_CB_METER_MOVED, &cb[0], 0);
    }
    widget = PtCreateWidget( PtMeter, 0, nargs, args );
}

void bah2() {
    PtWidget_t *widget;
    PtArg_t args[3];
    nargs = 0;
    if (true) {
        PtSetArg(&args[nargs++], Pt_ARG_AREA, &area, 0);
        PtSetArg(&args[nargs++], Pt_ARG_METER_KEY_LEFT, Pk_Down, 0);
        PtSetArg(&args[nargs++], Pt_ARG_METER_KEY_RIGHT, Pk_Up, 0);
    } else {
        PtSetArg(&args[nargs++], Pt_ARG_METER_INCREMENT, 10, 0);
        PtSetArg(&args[nargs++], Pt_CB_METER_MOVED, &cb[0], 0);
    }
    widget = PtCreateWidget( PtMeter, 0, nargs, args );
}

void bah3() {
    PtArg_t args[3];
    nargs = 0;
    if (true) {
        PtSetArg(&args[nargs++], Pt_ARG_AREA, &area, 0);
        PtSetArg(&args[nargs++], Pt_ARG_METER_KEY_LEFT, Pk_Down, 0);
        PtSetArg(&args[nargs++], Pt_ARG_METER_KEY_RIGHT, Pk_Up, 0);
    } else {
        PtSetArg(&args[nargs++], Pt_ARG_METER_INCREMENT, 10, 0);
        PtSetArg(&args[nargs++], Pt_CB_METER_MOVED, &cb[0], 0);
    }
    PtWidget_t *widget = PtCreateWidget( PtMeter, 0, nargs, args );
}
