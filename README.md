# IOKernelRW (한글 번역)

**IOKernelRW**는 macOS에서 커널 메모리 읽기/쓰기 권한을 제공하는 커널 확장(KEXT)입니다.



이 확장은 macOS의 보안을 **완전히 제거**합니다. 이게 바로 이 도구의 목적입니다. 운이 나쁘면 **시스템 안정성도 사라질 수 있습니다**.

---

## 🛠️ 빌드 방법

```bash
make
```

---

## 💻 설치 방법

1. **복구 모드(Recovery OS)**로 부팅하여 다음 명령어를 실행합니다:

```bash
bputil -k
csrutil disable
```

2. **macOS로 재부팅**한 후, 해당 디렉토리에서 다음 명령어를 실행합니다:

```bash
make install
```

3. macOS의 보안 경고/프롬프트에 따라 KEXT를 승인하고 다시 재부팅하세요.

> 만약 위 과정을 **되돌리고 싶다면**, macOS에서 인터넷이 연결된 상태에서 아래 명령어를 실행하고 재부팅하면 됩니다:

```bash
bputil -f
```

단, macOS 보안 정책을 따로 조정했다면, 해당 설정을 직접 복원할 수 있어야 합니다.

---

## 🚀 사용 방법

설치 후, 시스템의 IORegistry에 `IOKernelRW` 서비스 노드가 생성되며,  
이를 통해 `IOKernelRWUserClient`에 접근할 수 있습니다.  
단, 접근하려는 프로세스는 아래 entitlement가 필요합니다:

```
com.apple.security.siguza.kernel-rw
```

해당 userclient는 몇 가지 **외부 메서드(External Methods)**를 제공합니다.  
모두 scalar 인자만 받으며 반환값 외의 출력은 없습니다:

| ID | 이름         | Arg 0 (출발 주소) | Arg 1 (도착 주소)     | 길이  | Arg 3 (옵션)         |
|----|--------------|------------------|------------------------|-------|------------------------|
| 0  | readVirt     | 커널 가상주소     | 사용자 가상주소         | 길이  | 해당 없음              |
| 1  | writeVirt    | 사용자 가상주소   | 커널 가상주소           | 길이  | 해당 없음              |
| 2  | readPhys     | 물리 주소         | 사용자 가상주소         | 길이  | 정렬 방식 (4/8/0)¹     |
| 3  | writePhys    | 사용자 가상주소   | 물리 주소               | 길이  | 정렬 방식 (4/8/0)¹     |

¹ 정렬값이 4 또는 8이면 비캐시 접근이 강제되며, 해당 크기로 정렬됩니다.  
   값이 0이면 캐시 가능 영역으로 매핑되며, 정렬 제약이 없습니다.

> 편의성 있는 wrapper가 포함된 헤더 파일은 [`lib/iokernelrw.h`](https://github.com/ox1111/IOKernelRW/blob/master/lib/iokernelrw.h)에서 확인할 수 있습니다.

---

## 🔭 향후 계획 (Future Plans)

이 프로젝트의 목표는:

- **커널 LPE(Local Privilege Escalation)** 수준의 기능을 제공하고,
- **하드웨어 접근 및 리서치에 필요한 최소한의 primitive**를 구현하는 것입니다.

생각 중인 기능들:

- **사용자 공간으로 메모리 직접 매핑**  
  MIG 호출 없이 빠르게 접근 가능하지만, PPL 제약으로 기능에 한계가 있을 수 있습니다.

- **커널 함수 호출**  
  PAC 우회가 필요하므로 구현 난이도는 있음.

- **PAC 서명 위조**  
  커널 호출 primitive를 이용해 구현 가능성 있음.

- **커널 베이스 주소 유출**  
  물리 메모리에서 CTRR 레지스터나 IORVBAR 등을 읽는 방식으로 구현 가능성 있음.

---

## 📄 라이선스

MIT 라이선스를 따릅니다.  
자세한 내용은 [LICENSE](https://github.com/ox1111/IOKernelRW/blob/master/LICENSE) 파일을 참고하세요.





# IOKernelRW(영문버전)

Custom kext for kernel r/w on macOS.

If you don't know what that means, **stay away!** Dark things sleep in the abyss.

Needless to say, this will lower the security of your operating system to zero. That's the whole point. And with any luck it will also lower its stability to zero.

### Building

```
make
```

### Installation

1. Boot into recoveryOS and run:

   ```
   bputil -k
   csrutil disable
   ```
2. Reboot back into macOS and run (from within this folder):

   ```
   make install
   ```
3. Follow the OS prompts to approve the kext and reboot.

If you ever want to **undo** the above, you can just run `bputil -f` from within macOS (while connected to the internet) and reboot (unless you had previously lowered the security of your installation, in which case I trust you know how to adjust it back to how it was before).

### Usage

This creates an `IOKernelRW` service node in the IORegistry, on which an `IOKernelRWUserClient` can be opened by processes with the `com.apple.security.siguza.kernel-rw` entitlement.

The userclient exports a few external methods, all of which only take scalar inputs and provide no output (other than the return value). The following external methods exist:

| ID   | Name        | Arg 0                | Arg 1                     | Arg 2  | Arg 3                 |
| :--: | :---------- | :------------------- | :------------------------ | :----- | :-------------------- |
| 0    | `readVirt`  | source address (kVA) | destination address (uVA) | length | _N/A_                 |
| 1    | `writeVirt` | source address (uVA) | destination address (kVA) | length | _N/A_                 |
| 2    | `readPhys`  | source address (PA)  | destination address (uVA) | length | alignment<sup>1</sup> |
| 3    | `writePhys` | source address (uVA) | destination address (PA)  | length | alignment<sup>1</sup> |

<sup>1</sup> Values of 4 or 8 force all accesses to physical memory to be uncached and use the specified granularity as word size in bytes. Physical address and length must be a multiple of the chosen alignment. A value of 0 maps the physical range as cacheable instead, and puts no constraint on alignment or access granularity.

A header library with convenience wrappers exists in [`lib/iokernelrw.h`](https://github.com/Siguza/IOKernelRW/blob/master/lib/iokernelrw.h).

### Future Plans

The goal of this project is to provide capabilities that fall somewhere between what a kernel LPE would provide you and whatever primitives are useful for research and poking at hardware, with some degree of minimalism.

Things I thought about implementing:

- **Mapping memory into userspace**  
  Would remove the overhead of going through MIG and mapping/unmapping memory for each access, but is probably less powerful than direct accesses from kernel mode due to PPL constraints on what can be mapped into userspace? Would also require keeping track of `IOMemoryDescriptor` objects attached to the userclient, and that would require state management, reference counting, locking etc.
- **Kernel function calling**  
  Reasonable request. Might implement this if I ever feel like fiddling with PAC.
- **PAC signature forging**  
  Should be constructable with a kernel calling primitive.
- **Kernel base leak**  
  Should be constructable via phys read from something like CTRR lockdown registers or IORVBAR.

### License

[MIT](https://github.com/Siguza/IOKernelRW/blob/master/LICENSE).
