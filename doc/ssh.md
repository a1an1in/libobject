## v_2_1_0 



do_exec_pty

-->record_login

## refs

[The Secure Shell (SSH) Protocol Architecture](https://www.rfc-editor.org/rfc/rfc4251)

[The Secure Shell (SSH) Authentication Protocol](https://www.rfc-editor.org/rfc/rfc4252)

[The Secure Shell (SSH) Transport Layer Protocol](https://www.rfc-editor.org/rfc/rfc4253)

[The Secure Shell (SSH) Connection Protocol](https://www.rfc-editor.org/rfc/rfc4254)

[The Secure Shell (SSH) Public Key File Format](https://www.rfc-editor.org/rfc/rfc4716)

[Diffie-Hellman密码交换](https://www.zhihu.com/question/29383090/answer/70435297)

[重新认识SSH（一）](https://zhuanlan.zhihu.com/p/66058045)



## Q & As

* 加密和签名的区别？

  **数据加密：**用公钥加密，只能用自己的私钥解密，因为私钥只有你自己有，所以别人不可能能够解密，看到你的内容，保证了数据的保密性。

  **数据签名：**用私钥加密，只能用公钥解密，任何人都可以用公钥解密。因为私钥只有你自己有，所以保证了该数据肯定是从你这发送出去的，不可能是别人发的。

* 

## key words

output

send_context

cipher_set_key_iv

