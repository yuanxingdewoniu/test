/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/

asmlinkage long ( * old_sys_stat     )( char __user *filename, char __user *statbuf );
asmlinkage long ( * old_sys_stat_e32 )( char __user *filename, char __user *statbuf );

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
static inline long kernel_stat(
        char __user  *filename,
        char __user  *statbuf   )
{
    mm_segment_t fs_save;
    long or;
    
    fs_save = get_fs();
    set_fs( get_ds() );
    or = old_sys_stat( (const char __user *)filename, statbuf );
    set_fs( fs_save );
    
    //pr_dev( "Open DDDDD %ld\n", or );
    return or;
}

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
static inline long kernel_stat_e32(
        char __user  *filename,
        char __user  *statbuf       )
{
    mm_segment_t fs_save;
    long or;
    
    fs_save = get_fs();
    set_fs( get_ds() );
    or = old_sys_stat_e32( (const char __user *)filename, statbuf );
    set_fs( fs_save );

    //pr_dev( "Open DDDDD %ld\n", or );
    return or;
}

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
asmlinkage long efs_sys_stat_e32(
        char __user  *filename,
        char __user  *statbuf    ) 
{
    //if(!efs_netctrl_open_check(filename,flags,mode))return -EPERM;
    
    const char *redirected;
    
    int r = file_redirect_check( filename, &redirected, 1 );

    if ( r == OPEN_REDIRECTED_ALLOC )
    {
        long r1 = kernel_stat_e32( redirected, statbuf );
        kfree( redirected );
        return r1;
    }
    else if ( r == OPEN_REDIRECTED )
    {
        return  kernel_stat_e32( redirected, statbuf );
    }
    
    return old_sys_stat_e32( filename, statbuf );
}

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
asmlinkage long efs_sys_stat(
        char __user  *filename,
        char __user  *statbuf   ) 
{
    const char *redirected;
    
    int r = file_redirect_check( filename, &redirected, 0 );
    
    if ( r == OPEN_REDIRECTED_ALLOC )
    {
        long r1 = kernel_stat( redirected, statbuf );
        kfree( redirected );
        return r1;
    }
    else if ( r == OPEN_REDIRECTED )
    {
        return kernel_stat( redirected, statbuf );
    }
    
    return old_sys_stat( filename, statbuf );
}

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
