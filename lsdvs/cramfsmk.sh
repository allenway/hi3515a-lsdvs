#mv rootfs_uclibc_64k.jffs2 rootfs_uclibc_64k.jffs2$(date +%y%m%d%H%M%S)
echo "start mk cramfs for spi flash 64k block"
#strip bhdvs
cp rhadvs /install_sdk/Hi3520D_SDK_V1.0.1.0/package/rootfs_uclibc/tmp/dvs/
./mkfs.cramfs /install_sdk/Hi3520D_SDK_V1.0.1.0/package/rootfs_uclibc ./rootfs_uclibc_64k.cramfs 
#cp rootfs_uclibc_64k.cramfs /mnt/hgfs/F/tftp
echo "mk end"
