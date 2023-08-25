int ptconnect()
{
    PtAddCallback(ABW_CLstKvInstTRAJ, Pt_CB_SELECTION, (PtCallbackF_t *)cbSimpleClicLstFormatTraj, NULL);
    PtAddCallback(ABW_CValeurWrdTRAJ,Pt_CB_MODIFY_NOTIFY,(PtCallbackF_t *)cbModifCziTraj, NULL );
    PtAddCallback(ABW_CValeurWrdTRAJ,Pt_CB_TEXT_CHANGED,cbModifCziTraj, NULL );
    PtAddCallback(ABW_CBinstWaitComplement, Pt_CB_ACTIVATE, (PtCallbackF_t *)cbWaitSetTypeTest, NULL);
    PtAddCallback(ABW_CBtestPopup,Pt_CB_ARM,(PtCallbackF_t *)cbAppuiTestPopup, NULL );
    PtAddCallback(ABW_CBtestPopup, Pt_CB_DISARM, (PtCallbackF_t *)cbRelacherTestPopup, NULL);
    PtAddCallback( ABW_DtimerCleSecu, Pt_CB_TIMER_ACTIVATE, (PtCallbackF_t *)cb2TimerCleSecu, NULL );
    PtAddCallback( ABW_DtimerCleSecu, Pt_CB_TIMER_ACTIVATE, (PtCallbackF_t *)cb2TimerCleSecu, someData );
}
