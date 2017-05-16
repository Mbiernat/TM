int _system_pre_init(void)
{
    /* Insert your low-level initializations here */
    WDTCTL = WDTPW + WDTHOLD; // Stop Watchdog timer
    /*==================================*/
    /* Choose if segment initialization */
    /* should be done or not. */
    /* Return: 0 to omit initialization */
    /* 1 to run initialization */
    /*==================================*/
}

int main(void)
{
    return 0;
}
