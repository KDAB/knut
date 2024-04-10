void bah() {
    nargs = 0;
    PtSetArg(&args[nargs++], Pt_ARG_POS, &pos, 0);
    PtSetArg(&args[nargs++], ARG_MPEG_VITESSE, 250, 0);
    PtSetResources(widgetMpeg, nargs, args);
}
