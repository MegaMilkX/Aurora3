#ifndef PLATFORM_HPP
#define PLATFORM_HPP

bool platformInit();
void platformCleanup();
bool platformIsShuttingDown();
void platformUpdate();
void platformSwapBuffers();

void platformGetViewportSize(unsigned& width, unsigned& height);
void platformGetMousePos(unsigned& x, unsigned& y);
void* platformGetGlfwWindow();

#endif
