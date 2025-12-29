# Contributing to KINETIX

Thank you for your interest in contributing to **KINETIX**!  
We welcome contributions of all kinds — improvements to the 3D models, hardware design, firmware (C++), Android/Java apps, documentation, and bug fixes. To make it as easy as possible for you and for us to review your work, we ask that you follow these guidelines.

## 1. Getting Started

Before contributing, please:

1. Fork the repository and work on a branch named descriptively (e.g., `fix/servo-calibration`, `feature/ui-improvement`).  
2. Read existing documentation — README and any hardware/firmware docs — to understand the project structure.  
3. Search for existing issues to see if your contribution is already discussed.  
4. Discuss major changes on an issue before submitting a Pull Request (PR), especially hardware or API changes.

## 2. How to Contribute

### 🐛 Bug Reports & Feature Requests
- Use the *Issues* tab to report bugs or suggest improvements.
- Include clear steps to reproduce, environment info, and expected vs. actual behavior.

### ✨ Pull Requests (Code & Design)
PRs should:
1. Reference the relevant issue (e.g., “closes #42”).
2. Be atomic and focused — one feature or fix per PR.
3. Follow coding standards:
    - C++: consistent style, meaningful variable names, comment logic where needed.
    - Java: follow Android / Java conventions.
    - 3D models/PCB: include metadata (units, scale), and comments/labels where helpful.

### 🧱 3D Models & PCB Design
- Maintain consistent and clear file naming conventions.
- Use standard formats (e.g., STEP/IGES for mechanical, Eagle/KiCad for PCB) where practical.
- Ensure designs are printable/manufacturable with clear layer stacks and well-annotated drawings.
- Include rendered previews/screenshots for complex designs when possible.

## 3. Code & Hardware Quality Guidelines

- Documentation first: If code changes require new docs, update documentation in the same PR.  
- Test everything: Ensure that code compiles and firmware/hardware still functions as intended.  
- Respect API/HW compatibility: Major changes should be versioned and reviewed carefully.  
- Ensure backward compatibility as much as possible. Not all KinetiX devices run the same firmware version,
not all users run the same App version.
- Whatever your change does, extensively test the OTA feature is not broken. Breaking the OTA feature means
that once your version is installed on a device, it won't be OTA-updatable again and will require USB connection, some
updating software like esptool, which might be out of the reach of some users. 
- Label your commits clearly (e.g., `feat: add EEPROM config`, `fix: correct stepper direction`).  

## 4. Communication & Review
- Be responsive to review comments.
- Be respectful and collaborative — this is a shared non-profit open-source effort.

## 5. Licensing & Attribution
By contributing, you agree that your contributions will be licensed under the same terms as this project. If your contribution includes external designs or code, make sure they are compatible with this licensing.

---

Thank you for contributing — your improvements make KINETIX better for everyone!
