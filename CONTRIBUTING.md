# Contributing to HevSocks5Tunnel Android

Thank you for your interest in contributing to HevSocks5Tunnel Android! This document provides guidelines for contributing to the project.

## Code of Conduct

- Be respectful and inclusive
- Provide constructive feedback
- Focus on the issue, not the person
- Help make the community welcoming for everyone

## How to Contribute

### Reporting Bugs

Before creating a bug report:
1. Check existing issues to avoid duplicates
2. Use the latest version to verify the bug still exists
3. Collect relevant information (logs, device info, steps to reproduce)

When creating a bug report, include:
- Clear, descriptive title
- Detailed steps to reproduce
- Expected vs actual behavior
- Device/emulator information
- Android version
- Logs (use `adb logcat -s HevSocks5Tunnel HevSocks5TunnelJNI`)

### Suggesting Enhancements

Enhancement suggestions are welcome! Include:
- Clear description of the enhancement
- Use cases and benefits
- Possible implementation approach
- Any potential drawbacks

### Pull Requests

1. **Fork the repository**
   ```bash
   git clone https://github.com/YOUR_USERNAME/hev-socks5-tunnel-android.git
   ```

2. **Create a feature branch**
   ```bash
   git checkout -b feature/amazing-feature
   ```

3. **Make your changes**
   - Follow the coding standards (see below)
   - Write clear commit messages
   - Add tests if applicable
   - Update documentation

4. **Test your changes**
   ```bash
   ./gradlew build
   ./gradlew :app:connectedAndroidTest  # If you have tests
   ```

5. **Commit your changes**
   ```bash
   git commit -m "Add amazing feature"
   ```

6. **Push to your fork**
   ```bash
   git push origin feature/amazing-feature
   ```

7. **Create Pull Request**
   - Use a clear, descriptive title
   - Describe what changes you made and why
   - Reference any related issues
   - Include screenshots for UI changes

## Coding Standards

### Java Code Style

- Follow [Google Java Style Guide](https://google.github.io/styleguide/javaguide.html)
- Use meaningful variable and method names
- Add JavaDoc for public APIs
- Keep methods focused and concise
- Use proper exception handling

Example:
```java
/**
 * Starts the tunnel with the given configuration.
 *
 * @param config The tunnel configuration
 * @param tunFd The TUN interface file descriptor
 * @throws TunnelException If tunnel fails to start
 */
public void startAsync(TunnelConfig config, FileDescriptor tunFd) throws TunnelException {
    // Implementation
}
```

### C++ Code Style

- Follow [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- Use RAII principles
- Proper memory management (no leaks)
- Add comments for complex logic
- Use Android logging macros (LOGI, LOGE, etc.)

Example:
```cpp
/**
 * Starts the tunnel with the given configuration.
 * @param configPath Path to configuration file
 * @param tunFd TUN interface file descriptor
 * @return 0 on success, non-zero on error
 */
extern "C" JNIEXPORT jint JNICALL
Java_cc_hev_socks5_tunnel_HevSocks5Tunnel_nativeStart(
    JNIEnv *env, jobject thiz, jstring configPath, jint tunFd) {
    // Implementation
}
```

### Git Commit Messages

- Use present tense ("Add feature" not "Added feature")
- Use imperative mood ("Move cursor to..." not "Moves cursor to...")
- Limit first line to 72 characters
- Reference issues and PRs when relevant

Good examples:
```
Add support for SOCKS5 authentication
Fix memory leak in JNI layer
Update README with new API examples
Refactor tunnel configuration builder
```

### Testing

- Write tests for new features
- Ensure existing tests pass
- Test on multiple Android versions if possible
- Test on different ABIs (ARM, x86)

### Documentation

- Update README.md for API changes
- Add JavaDoc/comments for public APIs
- Update BUILD.md if build process changes
- Include usage examples

## Project Structure

```
hev-socks5-tunnel-android/
├── library/              # Main library module
│   ├── src/main/
│   │   ├── java/        # Java API layer
│   │   └── cpp/         # JNI wrapper
│   └── CMakeLists.txt   # Native build config
├── app/                 # Example application
│   └── src/main/
│       ├── java/        # Example VPN service
│       └── res/         # UI resources
├── docs/                # Additional documentation
└── BUILD.md            # Build instructions
```

## Development Setup

1. Install Android Studio
2. Install Android SDK (API 21-34)
3. Install NDK (25.1.8937393+)
4. Install CMake (3.22.1+)
5. Clone repository
6. Open in Android Studio
7. Sync Gradle
8. Build project

## Areas for Contribution

We welcome contributions in:

- **Bug fixes**: Fix issues reported in GitHub Issues
- **Features**: Implement requested features
- **Performance**: Optimize code performance
- **Documentation**: Improve docs and examples
- **Testing**: Add tests and improve coverage
- **Examples**: Add more usage examples
- **Compatibility**: Support more Android versions/devices

## Questions?

- Open a GitHub Issue for questions
- Check existing issues and documentation first
- Be specific and provide context

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

## Recognition

Contributors will be recognized in:
- GitHub contributors page
- Release notes for significant contributions
- README.md for major features

Thank you for contributing to HevSocks5Tunnel Android! 🎉
