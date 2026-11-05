.globl factorial

.data
n: .word 8

.text
main:
    la t0, n
    lw a0, 0(t0)   # a0有n 代表要帶入fun的para
    jal ra, factorial #存返回位置ra並跳到fac

    addi a1, a0, 0
    addi a0, x0, 1
    ecall # Print Result

    addi a1, x0, '\n'
    addi a0, x0, 11
    ecall # Print newline

    addi a0, x0, 10
    ecall # Exit

factorial:
    # YOUR CODE HERE
    addi t1, x0, 1  # t1存sum 值預設1
loop:
    ble a0, x0, end  # <=0 終止
    #開始計算fac
    mul t1, a0, t1
    addi a0, a0, -1
    j loop
end:
    addi a0, t1, 0  #儲存答案到a0(規定)
    jr ra

