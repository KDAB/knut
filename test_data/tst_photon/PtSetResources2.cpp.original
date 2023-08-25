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
    PtSetResources( widget, nargs, args );
}
