void foo()
{
    //Tworzenie okna.
    dim.w=600;
    dim.h=600;
    PtSetArg(&argwindow[0], Pt_ARG_DIM,&dim,0);
    PtSetArg(&argwindow[1], Pt_ARG_WINDOW_TITLE, "Modul Hefajstos" , 0);

    //tworzenie gridu
    dim.w=420;
    dim.h=420;
    PtSetArg(&arggrid[0], Pt_ARG_DIM,&dim,0);
    PtSetArg(&arggrid[1], Pt_ARG_GRID_VERTICAL,21,0);
    PtSetArg(&arggrid[2], Pt_ARG_GRID_HORIZONTAL,21,0);
    pol.x=10;
    pol.y=10;
    PtSetArg(&arggrid[3], Pt_ARG_POS,&pol,0);

    //koniec.
    window = PtCreateWidget(PtWindow, Pt_NO_PARENT, 2, argwindow);
    grid = PtCreateWidget(PtGrid, window,4, arggrid);


   //tree
    pol.x=440;
    pol.y=10;
    PtSetArg(&argtree[0], Pt_ARG_POS,&pol,0);
    dim.h=400;
    dim.w=200;
    PtSetArg(&argtree[1], Pt_ARG_DIM,&dim,0);

    PtSetArg(&argtree[2],Pt_ARG_SELECTION_MODE, Pt_SINGLE_MODE,0);

    tree= PtCreateWidget(PtList, window,4, argtree);



    //tree 2

    pol.x=650;
    pol.y=10;
    PtSetArg(&argtree2[0], Pt_ARG_POS,&pol,0);
    dim.h=400;
    dim.w=200;
    PtSetArg(&argtree2[1], Pt_ARG_DIM,&dim,0);

    // char * items="lipa";

    PtSetArg(&argtree2[2],Pt_ARG_SELECTION_MODE, Pt_SINGLE_MODE,0);
    // PtSetArg(&argtree2[3], Pt_ARG_ITEMS, &items, 1);

    tree2= PtCreateWidget(PtList, window,3, argtree2);
}
