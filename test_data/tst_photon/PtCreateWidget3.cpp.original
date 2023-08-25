void foo()
{
    PtWidget_t *containerOS_widgetMpeg;
    nargs = 0;
    PtSetArg(&args[nargs++], Pt_ARG_POS, &pos, 0);
    PtSetArg(&args[nargs++], Pt_ARG_DIM, &taille, 0);
    containerOS_widgetMpeg = PtCreateWidget(PtOSContainer, ABW_HCaScript, nargs, args);
}

void bar()
{
    PtWidget_t bouton[5];
    for (int i = 0; i < 5; ++i) {
        nargs = 0;
        PtSetArg( &args[nargs++], Pt_ARG_POS, &position, 0);
        PtSetArg( &args[nargs++], Pt_ARG_DIM, &taille, 0);
        PtSetArg( &args[nargs++], Pt_ARG_MARGIN_HEIGHT, 0, 0);
        PtSetArg( &args[nargs++], Pt_ARG_MARGIN_WIDTH, 0, 0);
        PtSetArg( &args[nargs++], Pt_ARG_BEVEL_WIDTH, 0, 0);
        bouton[i] = PtCreateWidget( BoutonDS, ABW_HCaScript, nargs, args);
    }
}
