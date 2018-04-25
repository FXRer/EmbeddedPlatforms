/*!
 * \brief Pure-virtual workaround.
 *
 * The avr-libc does not support a default implementation for handling 
 * possible pure-virtual calls. This is a short and empty workaround for this.
 */
extern "C" {
  void __cxa_pure_virtual()
  {
    // put error handling here
  }
}
