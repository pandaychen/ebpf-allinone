package main

import (
	"bytes"
	_ "embed"
	"encoding/binary"
	"log"
	"time"

	"github.com/cilium/ebpf/link"
	"github.com/cilium/ebpf/rlimit"

	"github.com/cilium/ebpf"
)

//https://github.com/cilium/ebpf/blob/main/examples/tracepoint_in_go/main.go

var mapKey uint32 = 9

// ebpf标签的说明
// BpfProg对应的是c程序中的bSEC(tracepoinst/...)的方法名
// TrackerMap对应的c程序中的SEC(map)
type bpfPrograms struct {
	BpfProg    *ebpf.Program `ebpf:"bpf_prog"`
	TrackerMap *ebpf.Map     `ebpf:"tracker_map"`
}

func (p *bpfPrograms) Close() error {
	err := p.BpfProg.Close()
	err = p.TrackerMap.Close()
	return err
}

// 字段类型，对应c程序中的event_data_t类型，注意Comm是uint8
type EventData struct {
	Pid  uint32
	Comm [20]uint8
}

func (e EventData) CommHex() string {
	ba := []byte{}
	for _, b := range e.Comm {
		ba = append(ba, b)
	}
	return string(ba)
}

// 程序
func LoadTestEbpf() {

	//
	if err := rlimit.RemoveMemlock(); err != nil {
		log.Fatal(err)
	}

	//加载c编译出来的.o文件的相对路径,这个地址一定不能出错
	spec, err := ebpf.LoadCollectionSpec("bpf_program.o")
	if err != nil {
		panic(err)
	}

	//赋值obj
	obj := bpfPrograms{}
	if err := spec.LoadAndAssign(&obj, nil); err != nil {
		panic(err)
	}
	defer obj.Close()

	tp, err := link.Tracepoint("syscalls", "sys_enter_execve", obj.BpfProg, nil)
	if err != nil {
		panic(err)
	}
	defer tp.Close()

	// Read loop reporting the total amount of times the kernel
	// function was entered, once per second.
	ticker := time.NewTicker(1 * time.Second)
	log.Println("Waiting for events..")
	for range ticker.C {

		var value []byte
		//c的结构体序列化成bytes（value）
		if err := obj.TrackerMap.Lookup(&mapKey, &value); err != nil {
			log.Printf("reading map: %v", err)
		} else {
			var event EventData
			//c的结构体序列化成golang定义的EventData结构
			if err := binary.Read(bytes.NewBuffer(value), binary.LittleEndian, &event); err != nil {
				log.Printf("parsing perf event: %s", err)
				continue
			}
			log.Printf("pid:%v,comm:%v", event.Pid, event.CommHex())
		}
	}
}

func main() {
	LoadTestEbpf()
}
