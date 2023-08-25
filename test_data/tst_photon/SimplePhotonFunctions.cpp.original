void foo()
{
    PtWidget_t *widget, *widget2;
    PgColor_t color;
    const char **listItemStrings;
    int i = 2;
    PhEvent_t *someEvent;
    PtRealizeWidget( widget );
    PtReparentWidget( widget, widget2 );
    PtSetResource( widget, Pt_ARG_FILL_COLOR, Pg_RED, 0);
    PtGetResource( widget, Pt_ARG_FILL_COLOR, &color, 0);
    PtListAddItems( widget, listItemStrings, 4, 0);
    PtListAddItems(  widget, listItemStrings, 4, i);
    PtListAddItems  ( widget, listItemStrings, i, 2);
    PtListSelectPos( widget, 3 );
    PtListSelectPos( ( PtWidget_t *   ) widget, i );
    PtListShowPos( widget, 1 );
    PtListShowPos(( PtWidget_t *   ) widget, i );
    PtListUnselectPos((PtWidget_t*)widget, 3 );
    PtListUnselectPos( widget, i );
    PtListDeleteAllItems( widget );
    PtHold();
    PtHold();
    PtStartFlux(widget);
    PtDamageWidget( widget );
    PtEndFlux(widget);
    PtRelease();
    PtUpdate();
    PtGiveFocus(widget, 0);
    PtGiveFocus(widget, someEvent);
    PtGiveContainerFocus(widget, someEvent);
    PtWindowFocus(widget);
    PtIsFocused  (widget);
    PtWindowToFront(widget);
    PtTreeItem_t *item;
    PtTreeExpand(widget, item, someEvent);
    if (PtWidgetIsArmed(widget))
        i = PtPGFindIndexByPanel( widget, widget2 );
    widget2 = PtPGFindPanelByIndex( widget, i );
    PtDestroyWidget( widget );
    PtBkgdHandlerProcess();
    if (PtWidgetIsClass(widget, PtLabel))
        return;

    char nomRes[20];
    ApDBase_t *ptrFPbitmaps = ApOpenDBase  ( (ApEventLink_t const *)ABM_FPbitmaps );
    sprintf(nomRes, "IconeDef" );
    PhImage_t *imageIconeDefaut = ApGetImageRes( ptrFPbitmaps, nomRes );
    sprintf(nomRes, "IconeWarn" );
    PhImage_t *imageIconeWarning = ApGetImageRes( ptrFPbitmaps, nomRes );
    PhImage_t *pImageBnStartG = ApGetImageRes  ( ptrFPbitmaps, "StartGoff" );
    PhImage_t *pImageBnStartG = ApGetImageRes( ptrFPbitmaps, "AcquitGoff" );
    PhImage_t *pImageBnStartD = ApGetImageRes  ( ptrFPbitmaps, "StartDoff" );
}
