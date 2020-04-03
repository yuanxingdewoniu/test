/******************************************************************************/
/*                                                                            */ 
/*                                                                            */
/*                                                                            */
/******************************************************************************/

#include "../../policy/policy.h"

extern void file_close_callback( struct file *filp );


asmlinkage long (*old_sys_open    )( const char __user *filename, int flags, int mode );
asmlinkage long (*old_sys_open_e32)( const char __user *filename, int flags, int mode );

//#if ( LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0) )
asmlinkage long (*old_sys_close    )( unsigned int fd );
asmlinkage long (*old_sys_close_e32)( unsigned int fd );
//#endif

/******************************************************************************/
/*                                                                            */ 
/*                                                                            */
/*                                                                            */
/******************************************************************************/
static inline long kernel_open_file( const char *path, int flags, umode_t mode )
{
    mm_segment_t fs_save;
    long or;
    
    fs_save = get_fs();

    set_fs( get_ds() );
    or = old_sys_open( (const char __user *)path, flags, mode );
    set_fs( fs_save );

    //pr_dev("Open DDDDD %ld\n",or);
    return or;
}

/******************************************************************************/
/*                                                                            */ 
/*                                                                            */
/*                                                                            */
/******************************************************************************/
static inline long kernel_open_file_e32( const char *path, int flags, int mode )
{
    mm_segment_t fs_save;
    long or;
    
    fs_save = get_fs();
     
    set_fs( get_ds() );
    or = old_sys_open_e32( (const char __user *)path, flags, mode );
    set_fs( fs_save );
     
    //pr_dev("Open DDDDD %ld\n",or);
    return or;
}

/******************************************************************************/
/*                                                                            */ 
/*                                                                            */
/*                                                                            */
/******************************************************************************/
asmlinkage long efs_sys_open_e32( const char __user *filename, int flags, int mode ) 
{
    //if(!efs_netctrl_open_check(filename,flags,mode))return -EPERM;

    const char *redirected;

    int r = file_redirect_check( filename, &redirected, 1 );

    if ( r == OPEN_REDIRECTED )
    {
        //printk("Kernel open e(32) redirected\n");
        return kernel_open_file_e32( redirected, flags, mode );
    }
    else if ( r == OPEN_REDIRECTED_ALLOC )
    {
        //printk("Kernel open e(32) redirected(allocted)\n");
        long r1 = kernel_open_file_e32( redirected, flags, mode );
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

        fd = old_sys_open_e32( filename, flags, mode );

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
    
    return old_sys_open_e32( filename, flags, mode );

    #endif      //
}

/******************************************************************************/
/*                                                                            */ 
// 功能: open系统调用的处理函数
/*                                                                            */
/******************************************************************************/
#if ( LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0) )
asmlinkage long efs_sys_open( const char __user *filename, int flags, int     mode ) 
#else
asmlinkage long efs_sys_open( const char __user *filename, int flags, umode_t mode ) 
#endif
{
    const char *redirected;

    // 1 重定向检查
    int r = file_redirect_check( filename, &redirected, 0 );

    // 2 如果要重定向，则打开重定向文件
    if ( r == OPEN_REDIRECTED )
    {
        return kernel_open_file( redirected, flags, mode );
    }
    else if ( r == OPEN_REDIRECTED_ALLOC )
    {
        long r1 =kernel_open_file( redirected, flags, mode );
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

        fd = old_sys_open( filename, flags, mode );

        if ( !efs_netctrl_open_check( filename, flags, mode ) )
        {
            old_sys_close( fd );
            return -EPERM;
        }

        return fd;
    }

    /**************************************************************************/
    //
    /**************************************************************************/
    #else

    // 3 网络控制
    if ( !efs_netctrl_open_check( filename, flags, mode ) )
        return -EPERM;

    // 4 打开原文件
    return old_sys_open( filename, flags, mode );

    #endif      //
}

/******************************************************************************/
//
// close
//
/******************************************************************************/
asmlinkage long efs_sys_close_e32( unsigned int fd )
{
    struct file *file;
    
    // 不处理特殊文件
    if ( fd == 0 || fd == 1 || fd == 2 )
        goto out;

    // 只处理访问过密文的程序
    if ( efs_test_accessed_encrypt_data() )
    {
        if ( file = fget( fd ) )
        {
            if ( ( file->f_flags & 0xF ) == O_RDONLY )      // O_RDONLY | O_WRONLY | O_RDWR
            {
                //pr_dev( "RDONLY : %u, %s, %s\n", fd, file->f_dentry->d_name.name, current_task_name() );
                ;           // do nothing !
            }
            else
            {
                //pr_dev( "------ : %u, %s, %s\n", fd, file->f_dentry->d_name.name, current_task_name() );

                if ( ( 0 != efs_policy_find_exceptredirection() ) ||
                     ( 0 != efs_policy_find_exceptpipe()        ) ||
                   //( 0 != efs_policy_find_nosavecontrol()     ) ||
                     ( 0 == efs_policy_find_contentfilter( current_task_name() ) ) )
                {
                    ;       // do nothing !
                }
                else
                {
                    file_close_callback( file );
                }
            }

            fput( file );
        }
    }

    /**************************************************************************/
    //
    /**************************************************************************/    
out:
    return old_sys_close_e32( fd );
}

/******************************************************************************/
//
// 
//
/******************************************************************************/
asmlinkage long efs_sys_close( unsigned int fd )
{


printk(" efs_sys_close  replace    \n ");
     return old_sys_close( fd );



    struct file *file;
    
    // 不处理特殊文件
    if ( fd == 0 || fd == 1 || fd == 2 )
        goto out;

    // 只处理访问过密文的程序
    if ( efs_test_accessed_encrypt_data() )
    {
        if ( file = fget( fd ) )
        {
            if ( ( file->f_flags & O_ACCMODE ) == O_RDONLY )
            {
                //pr_dev( "RDONLY : %u, %s, %s\n", fd, file->f_dentry->d_name.name, current_task_name() );
                ;           // do nothing !
            }
            else
            {
                //pr_dev( "------ : %u, %s, %s\n", fd, file->f_dentry->d_name.name, current_task_name() );

                if ( ( 0 != efs_policy_find_exceptredirection() ) ||
                     ( 0 != efs_policy_find_exceptpipe()        ) ||
                   //( 0 != efs_policy_find_nosavecontrol()     ) ||
                     ( 0 == efs_policy_find_contentfilter( current_task_name() ) ) )
                {
                    ;       // do nothing !
                }
                else
                {
                    file_close_callback( file );
                }
            }

            fput( file );
        }
    }

    /**************************************************************************/
    //
    /**************************************************************************/    
out:    
    return old_sys_close( fd );
}

/******************************************************************************/
//
// End of File
//
/******************************************************************************/
