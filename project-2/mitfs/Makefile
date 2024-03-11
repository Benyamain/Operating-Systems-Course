run:
	@echo "To quit, do 'ctrl a', then type 'c', then write 'q' once (qemu) shows up";
	qemu-system-riscv64 -machine virt -bios none -kernel kernel -m 128M -smp 4 -nographic -global virtio-mmio.force-legacy=false -drive file=fs.img,if=none,format=raw,id=x0 -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0 
