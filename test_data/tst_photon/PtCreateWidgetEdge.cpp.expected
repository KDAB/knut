void foo()
{
    //Tworzenie okna.
    dim.w=600;
    dim.h=600;
    window = new QMainWindow(0);
    window->resize(&dim);
    window->setWindowTitle("Modul Hefajstos");

    //tworzenie gridu
    dim.w=420;
    dim.h=420;
    grid = new Photon::Grid(window);
    grid->resize(&dim);
    grid->setVertical(21);
    grid->setHorizontal(21);
    pol.x=10;
    pol.y=10;
    grid->move(pol);

    //koniec.


   //tree
    pol.x=440;
    pol.y=10;
    tree = new Photon::List(window);
    tree->move(pol);
    dim.h=400;
    dim.w=200;
    tree->resize(&dim);

    tree->setSelectionModeFlags(QAbstractItemView::SingleSelection);




    //tree 2

    pol.x=650;
    pol.y=10;
    tree2 = new Photon::List(window);
    tree2->move(pol);
    dim.h=400;
    dim.w=200;
    tree2->resize(&dim);

    // char * items="lipa";

    tree2->setSelectionModeFlags(QAbstractItemView::SingleSelection);
    // PtSetArg(&argtree2[3], Pt_ARG_ITEMS, &items, 1);

}
