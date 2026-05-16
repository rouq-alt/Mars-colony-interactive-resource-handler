# Mars Colony Resource Manager - Font Customization

## Custom Font Support

This application supports custom fonts for enhanced Mars theme styling. Place font files in the `resources/` directory.

### Supported Font Files:
- `mars_font.ttf` - Main UI font (futuristic/sci-fi style recommended)
- `mars_mono.ttf` - Monospace font for technical data and numbers

### Font Recommendations for Mars Theme:
1. **Sci-Fi/Futuristic Fonts:**
   - Orbitron (free, geometric, space-themed)
   - Space Mono (monospace, clean)
   - Audiowide (bold, futuristic)
   - Exo (technical, modern)

2. **Where to Find Fonts:**
   - Google Fonts (free): fonts.google.com
   - DaFont (free): dafont.com
   - Font Squirrel (free): fontsquirrel.com

### Current Font Features:
- **Bold Text**: Created by drawing text multiple times with slight offsets
- **Outlined Text**: Black outline effect for important messages
- **Monospace Data**: Technical numbers and data use monospace font
- **Fallback**: Uses Raylib default font if custom fonts not found

### Adding Custom Fonts:
1. Download .ttf font files
2. Rename appropriately (`mars_font.ttf`, `mars_mono.ttf`)
3. Place in `resources/` directory
4. Recompile and run the application

The application will automatically detect and load custom fonts, falling back to defaults if not found.