void foo()
{
    Photon::Widget *widget, *widget2;
    QColor color;
    const char **listItemStrings;
    int i = 2;
    PhEvent_t *someEvent;
    widget->show();
    widget->setParent(widget2);
    widget->setFillColor(Qt::red);
    color = widget->fillColor();
    widget->insertItems(0, Util::toStringList(listItemStrings, 4));
    widget->insertItems(i, Util::toStringList(listItemStrings, 4));
    widget->insertItems(2, Util::toStringList(listItemStrings, i));
    widget->select(3);
    widget->select(i);
    widget->scrollTo( widget->model()->index(0, 0));
    widget->scrollTo( widget->model()->index(i - 1, 0));
    widget->deselect(3);
    widget->deselect(i);
    widget->deleteAllItems();
    //KDAB_NOTE: PtHold should not be needed, widget painting happens in the event loop
    //KDAB_NOTE: PtHold should not be needed, widget painting happens in the event loop
    //KDAB_NOTE: PtStartFlux should not be needed, widget painting happens in the event loop
    widget->update();
    //KDAB_NOTE: PtEndFlux should not be needed, widget painting happens in the event loop
    //KDAB_NOTE: PtRelease should not be needed, widget painting happens in the event loop
    //KDAB_NOTE: PtUpdate should not be needed, widget painting happens in the event loop
    widget->setFocus();
    widget->setFocus();
    widget->setFocus();
    widget->setFocus();
    widget->hasFocus() /*KDAB_NOTE: PtIsFocused had 3 return values.. 0: not focused, 1: "focus branch" (child is focused, !hasFocus()!!), 2: "focus leaf" (hasFocus) */;
    widget->raise() /*KDAB_NOTE: was PtWindowToFront call, verify that window doesn't get stuck behind another app window */;
    PtTreeItem_t *item;
    widget->expand(item);
    if (widget->isDown())
        i = widget->indexOf(widget2);
    widget2 = widget->widget(i);
    widget->destroy();
    QCoreApplication::processEvents();
    if (qobject_cast<Photon::Label*>(widget))
        return;

    char nomRes[20];
    ApDBase_t *ptrFPbitmaps = ApOpenDBase  ( (ApEventLink_t const *)ABM_FPbitmaps );
    sprintf(nomRes, "IconeDef" );
    QPixmap *imageIconeDefaut = QPixmap(":/KDAB_FIX_ptrFPbitmaps_res/" + nomRes + "-3001.png");
    sprintf(nomRes, "IconeWarn" );
    QPixmap *imageIconeWarning = QPixmap(":/KDAB_FIX_ptrFPbitmaps_res/" + nomRes + "-3001.png");
    QPixmap *pImageBnStartG = QPixmap(":/KDAB_FIX_ptrFPbitmaps_res/StartGoff-3001.png");
    QPixmap *pImageBnStartG = QPixmap(":/KDAB_FIX_ptrFPbitmaps_res/AcquitGoff-3001.png");
    QPixmap *pImageBnStartD = QPixmap(":/KDAB_FIX_ptrFPbitmaps_res/StartDoff-3001.png");
}
