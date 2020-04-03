/******************************************************************************/
/*                                                                            */ 
/*                                                                            */
/*                                                                            */
/******************************************************************************/

asmlinkage long (*old_sys_openat    )(          int dfd, const char __user *filename, int flags, int mode );
asmlinkage long (*old_sys_openat_e32)( unsigned int dfd, const char __user *filename, int flags, int mode );

/******************************************************************************/
/*                                                                            */ 
/*                                                                            */
/*                                                                            */
/******************************************************************************/
static inline long kernel_open_fileat( int dfd, const char __user *filename, int flags, int mode )
{
    mm_segment_t fs_save;
    long or;
         
    fs_save = get_fs();
     
    set_fs( get_ds() );
    or = old_sys_openat( dfd, (const char __user *)filename, flags, mode );
    set_fs( fs_save );
     
    //pr_dev("Open DDDDD %ld\n",or);

    return or;
}

/******************************************************************************/
/*                                                                            */ 
/*                                                                            */
/*                                                                            */
/******************************************************************************/
static inline long kernel_open_fileat_e32(unsigned int dfd, const char __user *filename, int flags, int mode)
{
    mm_segment_t fs_save;
    long or;
     
    fs_save = get_fs();
     
    set_fs( get_ds() );
    or = old_sys_openat_e32( dfd,(const char __user *)filename, flags, mode );
    set_fs( fs_save );

    //pr_dev("Open DDDDD %ld\n",or);

    return or;
}

/******************************************************************************/
/*                                                                            */ 
/*                                                                            */
/*                                                                            */
/******************************************************************************/
asmlinkage long efs_sys_openat_e32( unsigned int dfd, const char __user *filename, int flags, int mode )
{
    //if(!efs_netctrl_open_check(filename,flags,mode))return -EPERM;


    const char *redirected;

    int r = file_redirect_check( filename, &redirected, 1 );

    if ( r == OPEN_REDIRECTED )
    {
    	//printk( "Kernel openat e(32) redirected\n" );
    return kernel_open_fileat_e32( dfd, redirected, flags, mode );
    }
    else if( r == OPEN_REDIRECTED_ALLOC )
    {
    	//printk( "Kernel openat e(32) redirected(allocted)\n" );
    long r1 =kernel_open_fileat_e32( dfd, redirected, flags, mode );
    kfree( redirected );
    return r1;

    }
    else if ( r == OPEN_ERROR )
    {
    return -1;
    }

	/**************************************************************************/
    // LIN-87
    /**************************************************************************/
    #if ( LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0) )    

    {
        long fd = 0;

        fd = old_sys_openat_e32( dfd, filename, flags, mode );

        if ( !efs_netctrl_open_check( filename, flags, mode ) )
        {
            old_sys_close_e32( fd );
            return -EPERM;
        }

        return fd;
    }

    /**************************************************************************/
    // 
    /**************************************************************************/
    #else
    
    if ( !efs_netctrl_open_check( filename, flags, mode ) )
        return -EPERM;
    
    return old_sys_openat_e32( dfd, filename, flags, mode );

    #endif      //

}

/******************************************************************************/
/*                                                                            */ 
/*                                                                            */
/*                                                                            */
/******************************************************************************/
asmlinkage long efs_sys_openat( int dfd, const char __user *filename, int flags, int mode )
{
    const char *redirected;

    int r = file_redirect_check( filename, &redirected, 0 );

    if ( r == OPEN_REDIRECTED )
    {
    	//printk( "Kernel openat  redirected\n" );
    return kernel_open_fileat( dfd, redirected, flags, mode );
    }
    else if ( r == OPEN_REDIRECTED_ALLOC )
    {
    	//printk( "Kernel openat  redirected(allocted)\n" );
    long r1 = kernel_open_fileat( dfd, redirected, flags, mode );
    kfree( redirected );
    return r1;

    }
    else if ( r == OPEN_ERROR ) 
    {
    return -1;
    }

	/**************************************************************************/
    // LIN-87
    /**************************************************************************/
    #if ( LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0) )    

    long fd = 0;

    fd = old_sys_openat( dfd, filename, flags, mode );

    if ( !efs_netctrl_open_check( filename, flags, mode ) )
    {
        old_sys_close( fd );
        return -EPERM;
    }

    return fd;


    /**************************************************************************/
    //
    /**************************************************************************/
    #else

    // 3 ÍøÂç¿ØÖÆ
    if ( !efs_netctrl_open_check( filename, flags, mode ) )
        return -EPERM;

    // 4 Žò¿ªÔ­ÎÄŒþ
    return old_sys_openat( dfd, filename, flags, mode );

    #endif      //
	

}

/******************************************************************************/
/*                                                                            */ 
/*                                                                            */
/*                                                                            */
/******************************************************************************/
