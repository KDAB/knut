void bah() {
    PtArg_t args[5];
    nargs = 0;
    PtWidget_t *widget;
    PhDim_t dim;
    PhPoint_t pos;
    PtSetArg(&args[nargs++], Pt_ARG_POS, &pos, 0);
    PtSetArg(&args[nargs++], Pt_ARG_DIM, &dim, 0);
    //Using the non-pointer method
    PtSetArg(&args[nargs++], Pt_ARG_DIM, 0, 0);
    PtGetResources(widget, nargs, args);
}
