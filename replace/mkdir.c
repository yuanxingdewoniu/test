/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/

asmlinkage long (* old_sys_mkdir    )( const char __user *pathname,  int mode );
asmlinkage long (* old_sys_mkdir_e32)( const char __user *pathname,  u32 mode );

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
static inline long kernel_mkdir( const char __user *pathname, int mode )
{
    mm_segment_t fs_save;
    long or;

    fs_save = get_fs();
    set_fs( get_ds() );
    or = old_sys_mkdir( (const char __user *)pathname, mode );
    set_fs( fs_save );

    //pr_dev("Open DDDDD %ld\n",or);

    return or;
}

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
static inline long kernel_mkdir_e32( const char __user * pathname, u32 mode )
{
    mm_segment_t fs_save;
    long or;

    fs_save = get_fs();
    set_fs(get_ds());
    or = old_sys_mkdir_e32((const char __user *)pathname,mode);
    set_fs(fs_save);

    //pr_dev("Open DDDDD %ld\n",or);

    return or;
}

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
asmlinkage long efs_sys_mkdir_e32( const char __user * pathname, u32 mode ) 
{
    //if(!efs_netctrl_open_check(filename,flags,mode))return -EPERM;

    const char *redirected;

    int r = file_redirect_check( pathname, &redirected, 1 );

    if ( r == OPEN_REDIRECTED_ALLOC )
    {
        long r1 = kernel_mkdir_e32( redirected, mode );
        kfree( redirected );
        return r1;
    }
    else if( r == OPEN_REDIRECTED )
    {
        return  kernel_mkdir_e32( redirected, mode );
    }

    return old_sys_mkdir_e32( pathname, mode );
}

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
asmlinkage long efs_sys_mkdir( const char __user *pathname, int mode ) 
{
    const char *redirected;

    int r = file_redirect_check( pathname, &redirected, 0 );

    if ( r == OPEN_REDIRECTED_ALLOC )
    {
        long r1 = kernel_mkdir( redirected, mode );
        kfree( redirected );
        return r1;
    }
    else if( r == OPEN_REDIRECTED )
    {
        return  kernel_mkdir( redirected, mode );
    }

    return old_sys_mkdir( pathname, mode );
}

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
