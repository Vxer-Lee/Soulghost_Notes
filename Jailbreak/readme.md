# iOS越狱漏洞原理学习

## [Sock Port漏洞解析（一） UAF与Heap Spraying](https://mp.weixin.qq.com/s/R_KCNEpIn8O7ShTpnI0AMg)  

### #Userland Exploit
这类漏洞往往是对开源的Darwin-XNU进行代码审计发现的，基于这些漏洞往往能使我们在用户态将任意可执行代码送入内核执行。  
<br/>
<br/>
### #将用户态数据送人内核  
Userland Exploit能将任意数据写入到内核的堆区。 

骚操作：  
  Socket方式、Mach Message 和 IOSurface.
  ```c
  int sockt = socket(AF_INET6,socket_STREAM,IPOROTO_TCP);
  //在内核态会根据传入的参数创建 struct socket结构体
  struct socket{
      int so_zone;
      short so_type;
      u_hosrt so_error;
      u_int32_t so_options;
      short so_linger;
      short so_state;
      void *so_pcb;
  }
  ```
 `通过传入socket的参数，间接、受限制的控制了内核中的内存，但是只返回handle给我们，无法直接读取内核内存内容`
 怎么办😰？    
 <br/>

 ```c
 //借助socket options函数，可以修改socket一些配置.
int mintu = -1;
   //设置了IPV6下的最小mtu 为-1
   setsockopt(sock,IPPROTO_IPV6,IPV6_USE_MIN_MTU,&minmtu,sizeof(*mintu));
   //读取IPV6下的最小mtu
   getsockopt(sock,IPPROTO_IPV6,IPV6_USE_MIN_MTU,&minmtu,sizeof(*minmtu));
 ```  
**在内核态，系统会读取`struct socket`的so_pcb，并执行来自`用户态的读写操作`**
<br/>
<br/>

### #利用Socket 读写内核的任意内容
`伪造Socket结构体 利用setsockopt 和 getsockopt读写任意内存`
```objc
/*
 *iOS10.0 ~ 12.2 内核漏洞
 *漏洞名：Sock Port
 *原理：悬垂指针 ，只释放内存没有清理指针导致漏洞
*/
void in6_pcbdetach(strcut inpcb *inp)
{
    //...省略若干代码
    if(!(so->so_flags & SOF_PCBCLEARING))
    {
        //分别定义ip_moptions 和6的结构体，百度说是
        //传输层通过在这个结构包含的多播选项控制多播输出处理，
        //对每个输出的数据报，都可以ip_output传一个ip_moptions
        struct ip_moptions *imo;
        struct ip6_moptions *im6o;

        //Internet协议控制块(PCB) inp_vflag成员初始化
        inp->inp_vflag = 0;
        
        //绕过socke控制块的options不为空就释放掉内存
        if( inp->in6p_options != NULL)
        {
            //释放内存
            m_freem(inp->in6p_options);
            //养成好习惯，释放掉内存后，指针也要清零
            inp->in6p_options = NULL;//正常的操作（良好程序员👨‍💻‍）
        }

        //⚠️ 不合格👨‍💻‍ 写了这一句代码，简直是没有一点安全意识-----------------------------
        //漏洞原因：在进行资源释放的时候没有把inp->in6p_outputopts指向空
        ip6_freepcbopts(inp->in6p_outputopts);
        /*
        * ip6_freepcbopts 😳 代码
        *
        void ip6_freepcbopts(struct ip6_pktoppts *pktopt)
        {
            if(pktopt == NULL)
            {
                return;
            }
            //这里进行逐个释放，但是大佬说他忽略了上层，也就是指针本身！
            ip6_clearpkopts(pktopt,-1);
            FREE(PKTOPT,M_IP6OPT);
            //https://peterpan0927.github.io/2019/07/23/iOS12-2%E8%B6%8A%E7%8B%B1%E6%BC%8F%E6%B4%9E%E5%88%86%E6%9E%90/
            
        }*/

        //我看了一下ip6_freepcbopts这个函数，
        //他将in6p_outputopts中的资源逐个释放并指向空，但很可惜忽略了他的上层。
        //⚠️ ---------------------------------------------------------------------

        //释放 in6p_route
        ROUTE_RELEASE(&inp->in6p_route);

        //释放ipv4的资源
        if(inp->inp_options != NULL)
        {
            (void) m_free(inp->inp_options);
            inp->inp-options = NULL;//释放资源，并且将指针清零。良好程序员👩‍💻
        }

    }

    //...省略若干代码
    //只释放了in6p_outputopts，没有清理in6p_outputopts指针的地址，造成悬垂指针
}
```

<br/>
<br/>

### #悬垂指针
如上代码只释放了in6p_outputopts，没有清理in6p_outputopts指针的地址，造成`悬垂指针`
幸运的是我们可以用过 socket disconnect 后继续通过`setsockopt`和`getsockopt`间接读写这个悬垂指针。  
等待系统重新分配这块内存，我们就可以对其进行访问，因此转换成了如何间接控制系统对该区域的Reallocation。
<br/>
<br/>


### #Use After Free（悬垂指针操作已释放区域）
即尝试访问已释放的内存，这会导致程序崩溃，或是潜在的任意代码执行，甚至获取完全的远程控制能力。  
**UAF的关键之一是获取被释放区域的内存地址，一般透过悬垂指针实现，而悬垂指针是由于指针指向的内存区域被释放，但指针未被清零导致的。**  
<br/>
<br/>

### 堆喷射（Heap Sparaying）
简言之就是，比如我们 alloc 了 1 个 8B 的区域，随后将其释放，接下来再执行 alloc 时迟早会对先前的区域进行复用，如果恰好被我们 alloc 时占用，则达到了内容控制的目的。透过这种技术我们可以间接控制堆上的 Reallocation 内容。
<br/>
<br/>

### 堆风水（Heap feng-shui）
所谓堆风水也叫作堆排布，其实说严格了并不是一种漏洞的利用方法，而是一种灵活布置堆块来控制堆布局的方法，  
在一些一些其他漏洞的利用中起到效果。通过一道经典的题目，由清华蓝莲花战队出的babyfengshui来看一下  

[参考链接：堆风水](https://blog.csdn.net/Breeze_CAT/article/details/103788631)

<br/>
<br/>
  
### POC验证代码:  
(poc.cpp)[/Jailbreak/poc.cpp]